#pragma once

#include "net/data/UserInfo.hpp"
#include "net/worldstate/IWorldEventListener.hpp"

#include <vector>

class UserReporter final : IWorldEventListener
{
public:
	UserReporter(const std::shared_ptr<WorldState>& world);
	~UserReporter();

protected:
	void PostStringTableCreate(const std::shared_ptr<StringTable>& table) override;
	void PostStringTableUpdate(const std::shared_ptr<StringTable>& table) override;

private:
	void OnTableDataChanged(const std::shared_ptr<StringTable>& table);

	std::weak_ptr<WorldState> m_World;
	std::vector<UserInfo> m_Users;
};