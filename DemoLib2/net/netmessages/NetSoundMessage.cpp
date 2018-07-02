#include "NetSoundMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"

void NetSoundMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Reliable = reader.ReadBit();

	uint_fast16_t bitCount;
	if (m_Reliable)
	{
		m_SoundCount = 1;
		reader.Read(bitCount, RELIABLE_SIZE_BITS);
	}
	else
	{
		reader.Read(m_SoundCount, SOUND_COUNT_BITS);
		reader.Read(bitCount, UNRELIABLE_SIZE_BITS);
	}

	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetSoundMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Reliable);

	if (m_Reliable)
	{
		writer.Write(m_Data.Length().TotalBits(), RELIABLE_SIZE_BITS);
	}
	else
	{
		writer.Write(m_SoundCount, SOUND_COUNT_BITS);
		writer.Write(m_Data.Length().TotalBits(), UNRELIABLE_SIZE_BITS);
	}

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone);
}

void NetSoundMessage::GetDescription(std::ostream& description) const
{
	description << "svc_Sounds: number " << m_SoundCount;

	if (m_Reliable)
		description << ", reliable";

	description << ", bytes " << m_Data.Length().TotalBytes();
}
void NetSoundMessage::ApplyWorldState(WorldState& world) const
{
	// Sounds ignored for now
	//throw NotImplementedException();
}