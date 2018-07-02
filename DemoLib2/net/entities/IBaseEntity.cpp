#include "IBaseEntity.hpp"

#include "net/data/SendTable.hpp"
#include "net/data/ServerClass.hpp"

#include <algorithm>
#include <cassert>

IBaseEntity::IBaseEntity(const std::weak_ptr<WorldState>& world, const std::shared_ptr<const ServerClass>& serverClass, const std::shared_ptr<const SendTable>& networkTable) : m_World(world), m_ServerClass(serverClass), m_NetworkTable(networkTable)
{
	m_Properties.reserve(m_NetworkTable->GetFlattenedProperties().size());

	for (const auto& propDef : m_NetworkTable->GetFlattenedProperties())
		m_Properties.emplace_back(this, propDef);
}

bool IBaseEntity::Is(const std::string_view& name, bool recursive) const
{
	if (m_ServerClass->GetSendTable()->GetName() == name)
		return true;

	if (recursive)
	{
		for (auto base = m_ServerClass->GetSendTable()->GetBaseTable(); base; base = base->GetBaseTable())
		{
			if (base->GetName() == name)
				return true;
		}
	}

	return false;
}

void IBaseEntity::ResetProperties()
{
	for (auto& prop : m_Properties)
		prop.Clear();
}