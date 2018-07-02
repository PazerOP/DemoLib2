#include "NetPrefetchMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"

void NetPrefetchMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Type = PrefetchType::Sound;
	reader.Read<uint_fast16_t>("m_SoundIndex", m_SoundIndex, MAX_SOUND_INDEX_BITS);
}
void NetPrefetchMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_SoundIndex, MAX_SOUND_INDEX_BITS);
}

void NetPrefetchMessage::GetDescription(std::ostream& description) const
{
	description << "svc_Prefetch: type" << (int)m_Type << " index " << m_SoundIndex;
}
void NetPrefetchMessage::ApplyWorldState(WorldState& world) const
{
	// We don't really care about this since we're not playing sounds anyway
	//throw NotImplementedException();
}