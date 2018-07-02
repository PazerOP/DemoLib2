#include "NetSetConVarMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

void NetSetConVarMessage::ReadElementInternal(BitIOReader& reader)
{
	const auto count = reader.ReadInline<uint_fast8_t>(8);

	for (uint_fast8_t i = 0; i < count; i++)
	{
		auto name = reader.ReadString();
		auto value = reader.ReadString();
		m_Cvars.push_back(std::make_pair(name, value));
	}
}
void NetSetConVarMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Cvars.size(), 8);

	for (const auto& cvar : m_Cvars)
	{
		writer.Write(cvar.first);
		writer.Write(cvar.second);
	}
}

void NetSetConVarMessage::GetDescription(std::ostream& description) const
{
	description << "net_SetConVar: " << m_Cvars.size() << " cvars, \"" << m_Cvars[0].first << "\"=\"" << m_Cvars[0].second << '"';
}
void NetSetConVarMessage::ApplyWorldState(WorldState& world) const
{
	for (const auto& var : m_Cvars)
	{
		world.m_Events.ServerConVarSet(var.first, var.second);
		world.m_ConVars[var.first] = var.second;
	}
}