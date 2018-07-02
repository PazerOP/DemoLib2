#include "NetServerInfoMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

#include <cassert>

void NetServerInfoMessage::ReadElementInternal(BitIOReader& reader)
{
  reader.Read("NetServerInfoMessage::m_Protocol", m_Info.m_Protocol);
  reader.Read("NetServerInfoMessage::m_ServerCount", m_Info.m_ServerCount);
  reader.Read("NetServerInfoMessage::m_IsHLTV", m_Info.m_IsHLTV);
  reader.Read("NetServerInfoMessage::m_IsDedicated", m_Info.m_IsDedicated);
  reader.Read("NetServerInfoMessage::m_ClientCRC", m_Info.m_ClientCRC);
  reader.Read("NetServerInfoMessage::m_MaxClasses", m_Info.m_MaxClasses);

  // Unknown
  reader.ReadArray(m_Info.m_MapMD5, std::size(m_Info.m_MapMD5));

  reader.Read("NetServerInfoMessage::m_PlayerSlot", m_Info.m_PlayerSlot);
  reader.Read<uint8_t>("NetServerInfoMessage::m_MaxClients", m_Info.m_MaxClients);
  reader.Read<float>("NetServerInfoMessage::m_TickInterval", m_Info.m_TickInterval);

  const auto os = reader.ReadInline<char>();
  switch (os)
  {
    case 'l':
    case 'L':
      m_Info.m_OS = OperatingSystem::Linux;
      break;

    case 'w':
    case 'W':
      m_Info.m_OS = OperatingSystem::Windows;
      break;

    default:
      m_Info.m_OS = OperatingSystem::Unknown;
      assert(!"Unknown operating system!");
      break;
  }

  m_Info.m_GameDirectory = reader.ReadString("NetServerInfoMessage::m_GameDirectory");
  m_Info.m_MapName = reader.ReadString("NetServerInfoMessage::m_MapName");
  m_Info.m_SkyName = reader.ReadString("NetServerInfoMessage::m_SkyName");
  m_Info.m_Hostname = reader.ReadString("NetServerInfoMessage::m_Hostname");

  // Unknown
  reader.Read("NetServerInfoMessage::_unknown", m_Info._unknown);
}
void NetServerInfoMessage::WriteElementInternal(BitIOWriter& writer) const
{
  writer.Write(m_Info.m_Protocol);
  writer.Write(m_Info.m_ServerCount);
  writer.Write(m_Info.m_IsHLTV);
  writer.Write(m_Info.m_IsDedicated);
  writer.Write(m_Info.m_ClientCRC);
  writer.Write(m_Info.m_MaxClasses);

  // Unknown
  writer.WriteChars((char*)m_Info.m_MapMD5, std::size(m_Info.m_MapMD5));

  writer.Write(m_Info.m_PlayerSlot);
  writer.Write(m_Info.m_MaxClients);
  writer.Write(m_Info.m_TickInterval);

  if (m_Info.m_OS == OperatingSystem::Linux)
    writer.Write('l');
  else if (m_Info.m_OS == OperatingSystem::Windows)
    writer.Write('w');
  else
    writer.Write('?');  // Unknown operating system

  writer.Write(m_Info.m_GameDirectory);
  writer.Write(m_Info.m_MapName);
  writer.Write(m_Info.m_SkyName);
  writer.Write(m_Info.m_Hostname);

  writer.Write(m_Info._unknown);
}

void NetServerInfoMessage::GetDescription(std::ostream& description) const
{
  description << "svc_ServerInfo: game \"" << m_Info.m_GameDirectory <<
    "\", map \"" << m_Info.m_MapName << "\", max " << m_Info.m_MaxClients;
}
void NetServerInfoMessage::ApplyWorldState(WorldState& world) const
{
  world.m_ServerInfo = m_Info;
}