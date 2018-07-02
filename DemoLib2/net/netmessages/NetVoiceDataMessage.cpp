#include "NetVoiceDataMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"

void NetVoiceDataMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_ClientIndex, 8);
	reader.Read(_unknown, 7);
	reader.Read(m_Proximity);

	//for (uint_fast8_t i = 0; i < 8; i++)
	//  reader.DebugFind(6176 + i, 16, 32);

	const auto bitCount = reader.ReadInline<uint_fast32_t>(VOICE_DATA_LENGTH_BITS);
	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetVoiceDataMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_ClientIndex, 8);
	writer.Write(_unknown, 7);
	writer.Write(m_Proximity);

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(m_Data.Length().TotalBits(), VOICE_DATA_LENGTH_BITS);
	writer.Write(clone);
}

void NetVoiceDataMessage::GetDescription(std::ostream& description) const
{
	description << "svc_VoiceData: client " << +m_ClientIndex <<
		", bytes " << m_Data.Length().TotalBytes();
}
void NetVoiceDataMessage::ApplyWorldState(WorldState& world) const
{
	// Ignore voice data for now
	//throw NotImplementedException();
}