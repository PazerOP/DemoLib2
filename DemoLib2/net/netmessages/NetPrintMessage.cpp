#include "NetPrintMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

void NetPrintMessage::ReadElementInternal(BitIOReader& reader)
{
  m_Message = reader.ReadString("NetPrintMessage::m_Message");
}
void NetPrintMessage::WriteElementInternal(BitIOWriter& writer) const
{
  writer.Write(m_Message);
}

void NetPrintMessage::GetDescription(std::ostream& description) const
{
  description << "svc_Print: \"" << m_Message << '"';
}
void NetPrintMessage::ApplyWorldState(WorldState& world) const
{
  world.m_ServerText << m_Message;
  world.m_Events.ServerTextMessage(m_Message);
}