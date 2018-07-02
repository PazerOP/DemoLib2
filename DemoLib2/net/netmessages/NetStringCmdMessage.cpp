#include "NetStringCmdMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

#include <string_view>

void NetStringCmdMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Command = reader.ReadString();
}
void NetStringCmdMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Command);
}

void NetStringCmdMessage::GetDescription(std::ostream& description) const
{
	description << "net_StringCmd: \"" << m_Command << '"';
}
void NetStringCmdMessage::ApplyWorldState(WorldState& world) const
{
	bool isLastNewline = m_Command.back() == '\n';

	auto first = m_Command.find_first_of(' ');
	auto all = (std::string_view)m_Command;
	auto cmd = all.substr(0, first);
	auto args = all.substr(first + 1, all.size() - first - 1 - (isLastNewline ? 1 : 0));
	world.m_Events.ServerConCommand(cmd, args);
}