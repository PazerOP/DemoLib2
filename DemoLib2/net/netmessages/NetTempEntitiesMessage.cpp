#include "NetTempEntitiesMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/entities/EntityCoder.hpp"
#include "net/entities/TempEntity.hpp"
#include "net/worldstate/WorldState.hpp"

void NetTempEntitiesMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_EntryCount, EVENT_INDEX_BITS);

	auto bitCount = reader.ReadVarInt();
	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetTempEntitiesMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_EntryCount, EVENT_INDEX_BITS);

	writer.WriteVarInt(m_Data.Length().TotalBits());
	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone);
}

void NetTempEntitiesMessage::GetDescription(std::ostream& description) const
{
	description << "svc_TempEntities: number " << +m_EntryCount <<
		", bytes " << m_Data.Length().TotalBytes();
}
void NetTempEntitiesMessage::ApplyWorldState(WorldState& world) const
{
	auto clone = m_Data;
	std::shared_ptr<TempEntity> e;
	for (uint_fast8_t i = 0; i < m_EntryCount; i++)
	{
		float delay = 0;
		if (clone.ReadBit("TempEnt delay?"))
			delay = clone.ReadInline<uint8_t>("TempEnt delay centiseconds") / 100.0f;

		if (clone.ReadBit())
		{
			const auto serverClassID = clone.ReadInline(world.GetClassBits());
			const auto& serverClass = world.m_ServerClasses[serverClassID - 1];
			const auto& sendTable = serverClass->GetSendTable();

			if (e)
				world.m_Events.TempEntityCreated(e);	// We're done applying updates to a previous tempent

			e = std::make_shared<TempEntity>(world.GetWeak(), serverClass, sendTable);
			EntityCoder::ApplyEntityUpdate(*e, clone, world.m_Tick);
		}
		else
		{
			// Apply another update
			assert(e);
			EntityCoder::ApplyEntityUpdate(*e, clone, world.m_Tick);
		}
	}

	if (e)
		world.m_Events.TempEntityCreated(e);
}