#include "NetGetCvarValueMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "net/worldstate/WorldState.hpp"

#include <cmath>

void NetGetCvarValueMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_Cookie, COOKIE_BITS);
	m_CvarName = reader.ReadString(MAX_CVAR_NAME_SIZE);
}
void NetGetCvarValueMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Cookie, COOKIE_BITS);
	writer.Write(std::string_view(m_CvarName.c_str(), std::min<size_t>(m_CvarName.size(), MAX_CVAR_NAME_SIZE)));
}

void NetGetCvarValueMessage::GetDescription(std::ostream& description) const
{
	description << "svc_GetCvarValue: cvar: " << m_CvarName << ", cookie: " << m_Cookie;
}
void NetGetCvarValueMessage::ApplyWorldState(WorldState& world) const
{
	// Nothing to do
}