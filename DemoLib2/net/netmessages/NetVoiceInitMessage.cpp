#include "NetVoiceInitMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/netmessages/NetMessageType.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/worldstate/WorldState.hpp"

#include <cstring>

void NetVoiceInitMessage::ReadElementInternal(BitIOReader& reader)
{
  m_Setup.m_VoiceCodec = reader.ReadString();
  reader.Read(m_Setup.m_Quality, 8);

  if (m_Setup.m_Quality == QUALITY_HAS_SAMPLE_RATE)
    reader.Read(m_Setup.m_SampleRate, 16);
  else
    m_Setup.m_SampleRate = _stricmp(m_Setup.m_VoiceCodec.c_str(), "vaudio_celt") != 0 ? 11025 : 22050;
}
void NetVoiceInitMessage::WriteElementInternal(BitIOWriter& writer) const
{
  writer.Write(m_Setup.m_VoiceCodec);
  writer.Write(m_Setup.m_Quality, 8);

  if (m_Setup.m_Quality == QUALITY_HAS_SAMPLE_RATE)
    writer.Write(m_Setup.m_SampleRate, 16);
}

void NetVoiceInitMessage::GetDescription(std::ostream& description) const
{
  description << "svc_VoiceInit: codec \"" << m_Setup.m_VoiceCodec <<
    "\", qualitty " << +m_Setup.m_Quality;
}
void NetVoiceInitMessage::ApplyWorldState(WorldState& world) const
{
  auto clone = m_Setup;

  world.m_Events.PreVoiceSetupLoad(clone);
  world.m_VoiceSetup = std::move(clone);
  world.m_Events.PostVoiceSetupLoad();
}