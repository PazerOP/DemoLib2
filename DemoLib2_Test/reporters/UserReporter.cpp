#include "UserReporter.hpp"

#include "net/worldstate/WorldState.hpp"

#include <iostream>
#include <string.h>

UserReporter::UserReporter(const std::shared_ptr<WorldState>& world) : m_World(world)
{
	world->m_Events.AddEventListener(this);
}

UserReporter::~UserReporter()
{
	if (auto locked = m_World.lock())
		locked->m_Events.RemoveEventListener(this);

	for (const auto& user : m_Users)
	{
		cc::out << cc::fg::green << "UserID " << user.m_UserID << '\n'
			<< "\tName:               " << user.m_Name << '\n'
			<< "\tGUID:               " << user.m_GUID << '\n'
			<< "\tFriends ID:         " << user.m_FriendsID << '\n'
			<< "\tFake player:        " << std::boolalpha << user.m_IsFakePlayer << '\n'
			<< "\tHLTV:               " << user.m_IsHLTV << '\n'
			<< "\tCustom file CRC32s: " << std::hex << std::showbase << std::uppercase
			<< user.m_CustomFileCRC32s[0] << ", "
			<< user.m_CustomFileCRC32s[1] << ", "
			<< user.m_CustomFileCRC32s[2] << ", "
			<< user.m_CustomFileCRC32s[3] << '\n'
			<< std::dec
			<< "\tFiles downloaded:   " << user.m_FilesDownloaded
			<< cc::endl;
	}
}

void UserReporter::PostStringTableCreate(const std::shared_ptr<StringTable>& table)
{
	OnTableDataChanged(table);
}
void UserReporter::PostStringTableUpdate(const std::shared_ptr<StringTable>& table)
{
	OnTableDataChanged(table);
}
void UserReporter::OnTableDataChanged(const std::shared_ptr<StringTable>& table)
{
	if (_stricmp(table->GetName().c_str(), "userinfo"))
		return;

	for (const auto& entry : *table)
	{
		auto clone = entry.GetUserDataReader();
		clone.Seek(BitPosition::Zero(), Seek::Start);

		if (!clone.Length())
			continue;

		UserInfo user;
		user.ReadElement(clone);

		auto found = std::find_if(m_Users.begin(), m_Users.end(),
			[&user](const UserInfo& a) { return !_stricmp(a.m_GUID, user.m_GUID); });
		if (found == m_Users.end())
			m_Users.push_back(user);
		else
			*found = user;
	}
}