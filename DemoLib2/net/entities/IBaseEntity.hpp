#pragma once

#include "net/entities/IPropertySet.hpp"

#include <memory>

class SendTable;
class ServerClass;
class WorldState;

class IBaseEntity : public IPropertySet
{
public:
	IBaseEntity(const std::weak_ptr<WorldState>& world, const std::shared_ptr<const ServerClass>& serverClass, const std::shared_ptr<const SendTable>& networkTable);
	virtual ~IBaseEntity() = default;

	auto GetWorld() const { return m_World.lock(); }

	const auto& GetServerClass() const { return m_ServerClass; }
	const auto& GetNetworkTable() const { return m_NetworkTable; }

	bool Is(const std::string_view& name, bool recursive = true) const;

	void ResetProperties();

	virtual bool IsTempEntity() const = 0;

private:
	std::weak_ptr<WorldState> m_World;
	std::shared_ptr<const SendTable> m_NetworkTable;
	std::shared_ptr<const ServerClass> m_ServerClass;
};