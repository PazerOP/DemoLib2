#include "NetEntityMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"

void NetEntityMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_EntityIndex, MAX_EDICT_BITS);
	reader.Read(m_ClassID, MAX_SERVER_CLASS_BITS);

	const auto bitCount = reader.ReadInline<uint_fast32_t>(MAX_ENTITY_MSG_LENGTH_BITS);
	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetEntityMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_EntityIndex, MAX_EDICT_BITS);
	writer.Write(m_ClassID, MAX_SERVER_CLASS_BITS);

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone.Length().TotalBits(), MAX_ENTITY_MSG_LENGTH_BITS);
	writer.Write(clone);
}

void NetEntityMessage::GetDescription(std::ostream& description) const
{
	description << "svc_EntityMessage: entity " << m_EntityIndex <<
		", class " << m_ClassID << ", bytes " << m_Data.Length().TotalBytes();
}
void NetEntityMessage::ApplyWorldState(WorldState& world) const
{
	// Like usermessages, we're ignoring these for now.
	//throw NotImplementedException();
}