#include "NetSetViewMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/worldstate/WorldState.hpp"

#include <inttypes.h>

void NetSetViewMessage::ReadElementInternal(BitIOReader& reader)
{
  reader.Read(m_ViewEntIndex, MAX_EDICT_BITS);
}
void NetSetViewMessage::WriteElementInternal(BitIOWriter& writer) const
{
  writer.Write(m_ViewEntIndex, MAX_EDICT_BITS);
}

void NetSetViewMessage::GetDescription(std::ostream& description) const
{
  description << "svc_SetView: view entity " << m_ViewEntIndex;
}
void NetSetViewMessage::ApplyWorldState(WorldState& world) const
{
  auto clone = m_ViewEntIndex;

  bool shouldUpdate = true;
  world.m_Events.PreViewEntityUpdate(shouldUpdate, clone);

  if (shouldUpdate)
  {
    world.m_ViewEntity = clone;
    world.m_Events.PostViewEntityUpdate();
  }
}