#include "NetUsrMsgMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"

void NetUsrMsgMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_Type, MAX_USER_MSG_TYPE_BITS);

	auto bitCount = reader.ReadInline<uint_fast16_t>(MAX_USER_MSG_LENGTH_BITS);
	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetUsrMsgMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Type, MAX_USER_MSG_TYPE_BITS);

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(m_Data.Length().TotalBits(), MAX_USER_MSG_LENGTH_BITS);
	writer.Write(clone);
}

void NetUsrMsgMessage::GetDescription(std::ostream& description) const
{
	description << "svc_UserMessage: type " << (int)m_Type <<
		", bytes " << m_Data.Length().TotalBytes();
}
void NetUsrMsgMessage::ApplyWorldState(WorldState& world) const
{
	// Don't process usermessages for now
	//throw NotImplementedException();
}