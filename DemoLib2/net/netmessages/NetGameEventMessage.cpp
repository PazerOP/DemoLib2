#include "NetGameEventMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

#include <iostream>

void NetGameEventMessage::ReadElementInternal(BitIOReader& reader)
{
	auto bitCount = reader.ReadInline<uint_fast16_t>(EVENT_LENGTH_BITS);
	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetGameEventMessage::WriteElementInternal(BitIOWriter& writer) const
{
	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(m_Data.Length().TotalBits(), EVENT_LENGTH_BITS);
	writer.Write(clone);
}

void NetGameEventMessage::GetDescription(std::ostream& description) const
{
	description << "svc_GameEvent: bytes " << m_Data.Length().TotalBytes();
}
void NetGameEventMessage::ApplyWorldState(WorldState& world) const
{
	if (world.m_GameEventDeclarations.empty())
	{
		cc::out << "Discarding game event, game event declarations not loaded yet" << std::endl;
		return;
	}

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);

	const auto eventID = clone.ReadInline(EVENT_ID_BITS);
	const auto& eventDeclaration = world.m_GameEventDeclarations[eventID];

	auto event = std::make_shared<GameEvent>(eventDeclaration);

	for (const auto& key : eventDeclaration.m_Values)
	{
		switch (key.second)
		{
		case GameEventDataType::Local: break;	// Ignore

		case GameEventDataType::String:
			event->Set(key.first, clone.ReadString());
			break;

		case GameEventDataType::Float:
			event->Set(key.first, clone.ReadInline<float>());
			break;

		case GameEventDataType::Long:
			event->Set(key.first, clone.ReadInline<int>());
			break;

		case GameEventDataType::Short:
			event->Set(key.first, clone.ReadInline<short>());
			break;

		case GameEventDataType::Byte:
			event->Set(key.first, clone.ReadInline<uint8_t>());
			break;

		case GameEventDataType::Bool:
			event->Set(key.first, clone.ReadBit());
			break;

		default:
			throw std::logic_error("Unknown GameEventDataType");
		}
	}

	world.m_Events.GameEventFired(event);
}