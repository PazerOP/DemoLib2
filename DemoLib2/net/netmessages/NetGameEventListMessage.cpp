#include "NetGameEventListMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/worldstate/WorldState.hpp"

void NetGameEventListMessage::ReadElementInternal(BitIOReader& reader)
{
	const auto eventsCount = reader.ReadInline<uint_fast16_t>("eventsCount", MAX_EVENT_BITS);

	reader.Read("m_BitCount", m_BitCount, BIT_COUNT_BITS);

	for (uint_fast16_t i = 0; i < eventsCount; i++)
	{
		GameEventDeclaration& e = m_Events.emplace_back();

		reader.Read("m_ID", e.m_ID, MAX_EVENT_BITS);
		e.m_Name = reader.ReadString("m_Name");

		GameEventDataType type;
		while ((type = reader.ReadInline<GameEventDataType>("GameEventDataType", EVENT_DATA_TYPE_BITS)) != GameEventDataType::Local)
		{
			const auto name = reader.ReadString("name");
			e.m_Values.push_back(std::make_pair(name, type));
		}
	}
}
void NetGameEventListMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Events.size(), MAX_EVENT_BITS);

	writer.Write(m_BitCount, BIT_COUNT_BITS);

	for (const auto& event : m_Events)
	{
		writer.Write(event.m_ID, MAX_EVENT_BITS);
		writer.Write(event.m_Name);

		for (const auto& dt : event.m_Values)
		{
			writer.Write(dt.second, EVENT_DATA_TYPE_BITS);
			writer.Write(dt.first);
		}

		writer.Write(GameEventDataType::Local, EVENT_DATA_TYPE_BITS);
	}
}

void NetGameEventListMessage::GetDescription(std::ostream& description) const
{
	description << "svc_GameEventList: number " << m_Events.size() << ", bytes " << (m_BitCount + 7) / 8;
}
void NetGameEventListMessage::ApplyWorldState(WorldState& world) const
{
	auto clone = m_Events;

	world.m_Events.PreGameEventListLoad(clone);
	world.m_GameEventDeclarations = std::move(clone);
	world.m_Events.PostGameEventListLoad();
}