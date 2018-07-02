#include "NetSignonStateMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

void NetSignonStateMessage::ReadElementInternal(BitIOReader& reader)
{
  reader.Read<ConnectionState>(m_State.m_State, 8);
  reader.Read(m_State.m_SpawnCount);
}
void NetSignonStateMessage::WriteElementInternal(BitIOWriter& writer) const
{
  writer.Write(m_State.m_State, 8);
  writer.Write(m_State.m_SpawnCount);
}

void NetSignonStateMessage::GetDescription(std::ostream& description) const
{
  description << "net_SignonState: state " << (int)m_State.m_State <<
    ", count " << m_State.m_SpawnCount;
}
void NetSignonStateMessage::ApplyWorldState(WorldState& world) const
{
  world.SetSignonState(m_State);
}