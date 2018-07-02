#include "NetPacketEntitiesMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SendPropDefinition.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/entities/Entity.hpp"
#include "net/entities/EntityCoder.hpp"
#include "net/worldstate/WorldState.hpp"

#include <iostream>
#include <string_view>

void NetPacketEntitiesMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_MaxEntries, MAX_EDICT_BITS);

	reader.Read(m_IsDelta);
	if (m_IsDelta)
		reader.Read(m_DeltaFrom, DELTA_INDEX_BITS);

	reader.Read(m_Baseline, 1);

	reader.Read(m_UpdatedEntries, MAX_EDICT_BITS);

	auto bitCount = reader.ReadInline<uint_fast32_t>(DELTA_SIZE_BITS);

	reader.Read(m_UpdateBaseline);

	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetPacketEntitiesMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_MaxEntries, MAX_EDICT_BITS);

	writer.Write(m_IsDelta);
	if (m_IsDelta)
		writer.Write(m_DeltaFrom.value(), DELTA_INDEX_BITS);

	writer.Write(m_Baseline, 1);

	writer.Write(m_UpdatedEntries, MAX_EDICT_BITS);

	writer.Write(m_Data.Length().TotalBits(), DELTA_SIZE_BITS);

	writer.Write(m_UpdateBaseline);

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone);
}

void NetPacketEntitiesMessage::GetDescription(std::ostream& description) const
{
	description << "svc_PacketEntities: delta " << (m_DeltaFrom.has_value() ? m_DeltaFrom.value() : -1) <<
		", max " << m_MaxEntries <<
		", changed " << m_UpdatedEntries;

	if (m_UpdateBaseline)
		description << " BL update,";

	description << " bytes " << m_Data.Length().TotalBytes();
}

void NetPacketEntitiesMessage::ApplyWorldState(WorldState& world) const
{
	if (world.m_SignonState->m_State == ConnectionState::Spawn)
	{
		if (!m_IsDelta)
		{
			// We are done with the signon sequence.
			auto clone = world.m_SignonState.value();
			clone.m_State = ConnectionState::Full;
			world.SetSignonState(clone);
		}
		else
			throw ParseException("Received delta packet entities while spawning!");
	}

	if (m_IsDelta)
	{
		if (world.m_Tick == m_DeltaFrom)
			throw ParseException("Update self-referencing");
	}

	if (m_UpdateBaseline)
	{
		auto& bl0 = world.m_InstanceBaselines[(int)BaselineIndex::Baseline0];
		auto& bl1 = world.m_InstanceBaselines[(int)BaselineIndex::Baseline1];

		if (m_Baseline == BaselineIndex::Baseline0 || m_Baseline == BaselineIndex::Baseline1)
		{
			for (size_t i = 0; i < std::size(bl0); i++)
				std::swap(bl0[i], bl1[i]);
		}
		else
		{
			std::stringstream temp;
			temp << "Unknown baseline index " << (int)m_Baseline;
			throw ParseException(temp.str());
		}
	}

	auto dataClone = m_Data;
	dataClone.Seek(BitPosition::Zero(), Seek::Start);

	int newEntIndex = -1;
	for (uint_fast16_t i = 0; i < m_UpdatedEntries; i++)
	{
		newEntIndex += 1 + EntityCoder::ReadUBitVar(dataClone);

		if (!dataClone.ReadBit("Leave PVS Flag"))
		{
			if (dataClone.ReadBit("Enter PVS Flag"))
			{
				auto newEnt = ReadEnterPVS(world, dataClone, newEntIndex);

				const auto startPos = dataClone.GetPosition();
				EntityCoder::ApplyEntityUpdate(*newEnt, dataClone, world.m_Tick);
				const auto endPos = dataClone.GetPosition();

				const bool isNewlyCreated = world.m_Entities[newEntIndex] != newEnt;
				std::weak_ptr<Entity> old = world.m_Entities[newEntIndex];
				world.m_Entities[newEntIndex] = newEnt;
				assert(!isNewlyCreated || old.expired());

				if (m_UpdateBaseline)
				{
					auto& target = world.m_InstanceBaselines[(int)m_Baseline == 0 ? 1 : 0][newEntIndex];
					target = dataClone.Span(startPos, endPos);
				}

				// If we were just created, we don't have to worry about any event listeners existing on the ent
				newEnt->SetInPVS(true);

				if (isNewlyCreated)
					world.m_Events.EntityCreated(newEnt);

				world.m_Events.EntityEnteredPVS(newEnt);
			}
			else
			{
				// Preserve/update
				const auto& ent = world.m_Entities[newEntIndex];

				if (EntityCoder::ApplyEntityUpdate(*ent, dataClone, world.m_Tick))
					world.m_Events.EntityUpdated(ent);  // Tell the world
			}
		}
		else
		{
			bool shouldDelete = dataClone.ReadBit("'Should delete' Flag");

			assert(m_IsDelta);

			ReadLeavePVS(world, newEntIndex, shouldDelete);
		}
	}

	if (m_IsDelta)
	{
		// Read explicit deletions
		while (dataClone.ReadBit())
		{
			uint_fast16_t ent = dataClone.ReadInline<uint16_t>(MAX_EDICT_BITS);
			world.m_Entities[ent].reset();
		}
	}

	assert(!dataClone.Remaining());
}

std::shared_ptr<Entity> NetPacketEntitiesMessage::ReadEnterPVS(WorldState& world, BitIOReader& reader, uint_fast16_t entityIndex) const
{
	const auto serverClassIndex = reader.ReadInline(world.GetClassBits());
	const auto& serverClass = world.m_ServerClasses[serverClassIndex];
	const auto& networkTable = serverClass->GetSendTable();

	const auto serialNumber = reader.ReadInline(NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS);

	auto ent = world.m_Entities[entityIndex];
	bool isNew = false;
	if (!ent || ent->GetSerialNumber() != serialNumber)
	{
		// No existing entity, or we're replacing an existing entity in this slot
		ent = std::make_shared<Entity>(world.GetWeak(), serverClass, networkTable, entityIndex, serialNumber);

		// Clear instance baselines for this entindex
		world.m_InstanceBaselines[0][entityIndex].reset();
		world.m_InstanceBaselines[1][entityIndex].reset();

		isNew = true;
	}

	if (auto instanceBL = world.m_InstanceBaselines[(int)m_Baseline][entityIndex])
	{
		assert(!instanceBL->GetLocalPosition());

		if (!isNew)
			ent->ResetProperties();

		EntityCoder::ApplyEntityUpdate(*ent, *instanceBL, 0);
		assert(!instanceBL->Remaining());
	}
	else if (auto staticBaseline = world.GetStaticBaseline(serverClassIndex))
	{
		// No previous instance-specific baseline, try for static baseline
		assert(!staticBaseline->GetLocalPosition());

		if (!isNew)
			ent->ResetProperties();

		EntityCoder::ApplyEntityUpdate(*ent, *staticBaseline, 0);
		assert(staticBaseline->Remaining() < BitPosition::FromBytes(1));
	}
	else
	{
		cc::out << "No instance or static baseline found for entity #" << entityIndex << " (" << serverClass->GetClassname() << ')' << std::endl;
	}

	return ent;
}

void NetPacketEntitiesMessage::ReadLeavePVS(WorldState& world, uint_fast16_t entIndex, bool bDelete) const
{
	auto& ent = world.m_Entities[entIndex];

	if (ent)
	{
		ent->SetInPVS(false);
		world.m_Events.EntityLeftPVS(ent);

		if (bDelete)
		{
			ent->OnDeleted(ent);
			world.m_Events.EntityDeleted(ent);
			std::weak_ptr<Entity> weak = ent;
			ent.reset();
			//assert(weak.expired());
		}
	}
}
