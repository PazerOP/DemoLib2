#include "NetFileMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"

void NetFileMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_TransferID, 32);
	m_Filename = reader.ReadString();
	reader.Read(m_Status, 1);
}
void NetFileMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_TransferID, 32);
	writer.Write(m_Filename);
	writer.Write(m_Status, 1);
}

void NetFileMessage::GetDescription(std::ostream& description) const
{
	description << "net_File: " << m_Filename << ' ' << (int)m_Status;
}
void NetFileMessage::ApplyWorldState(WorldState& world) const
{
	//throw NotImplementedException();
}