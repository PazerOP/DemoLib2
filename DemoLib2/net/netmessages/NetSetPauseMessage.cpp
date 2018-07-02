#include "NetSetPauseMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "net/worldstate/WorldState.hpp"

void NetSetPauseMessage::GetDescription(std::ostream& description) const
{
	description << "svc_SetPause: " << (m_Paused ? "paused" : "unpaused");
}

void NetSetPauseMessage::ApplyWorldState(WorldState& world) const
{
	world.m_Paused = m_Paused;
}

void NetSetPauseMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Paused = reader.ReadBit("is paused?");
}

void NetSetPauseMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Paused);
}
