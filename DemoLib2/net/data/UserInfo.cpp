#include "UserInfo.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"

void UserInfo::ReadElementInternal(BitIOReader& reader)
{
	reader.ReadArray(m_Name, std::size(m_Name));
	reader.Read("m_UserID", m_UserID);
	reader.ReadArray(m_GUID, std::size(m_GUID));

	reader.Read("m_FriendsID", m_FriendsID);
	reader.ReadArray(m_FriendsName, std::size(m_FriendsName));

	reader.Read<uint8_t>(*(uint8_t*)&m_IsFakePlayer);
	reader.Read<uint8_t>(*(uint8_t*)&m_IsHLTV);

	for (uint_fast8_t i = 0; i < std::size(m_CustomFileCRC32s); i++)
		reader.Read(m_CustomFileCRC32s[i]);

	reader.Read(m_FilesDownloaded);
}
void UserInfo::WriteElementInternal(BitIOWriter& writer) const
{
	writer.WriteChars(m_Name, sizeof(m_Name));
	writer.Write(m_UserID);
	writer.WriteChars(m_GUID, sizeof(m_GUID));

	writer.Write(m_FriendsID);
	writer.WriteChars(m_FriendsName, sizeof(m_FriendsName));

	writer.Write<uint8_t>(*(uint8_t*)&m_IsFakePlayer);
	writer.Write<uint8_t>(*(uint8_t*)&m_IsHLTV);

	for (uint_fast8_t i = 0; i < std::size(m_CustomFileCRC32s); i++)
		writer.Write(m_CustomFileCRC32s[i]);

	writer.Write(m_FilesDownloaded);
}