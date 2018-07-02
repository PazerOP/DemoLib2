#pragma once

#include "BitIO/IStreamElement.hpp"

#include <string>

class UserInfo final : public IStreamElement
{
public:
	char m_Name[32];
	uint32_t m_UserID;
	char m_GUID[33];

	uint32_t m_FriendsID;
	char m_FriendsName[32];

	bool m_IsFakePlayer;
	bool m_IsHLTV;

	uint32_t m_CustomFileCRC32s[4];
	uint32_t m_FilesDownloaded;

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const { return new UserInfo(); }
};