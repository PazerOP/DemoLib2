#include "Entity.hpp"

#include "net/data/SendPropDefinition.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/worldstate/WorldState.hpp"

#include <algorithm>
#include <cassert>

Entity::Entity(const std::weak_ptr<WorldState>& world, const std::shared_ptr<const ServerClass>& serverClass, const std::shared_ptr<const SendTable>& networkTable, uint_fast16_t entityIndex, uint_fast16_t serialNumber) : IBaseEntity(world, serverClass, networkTable)
{
	m_Index = entityIndex;
	m_SerialNumber = serialNumber;
	m_InPVS = false;

	if (auto realWorld = world.lock())
		m_CreatedTick = realWorld->m_Tick;

	m_LastPVSStateChangeTick = 0;
}

void Entity::SetInPVS(bool inPVS)
{
	bool oldInPVS = m_InPVS;
	m_InPVS = inPVS;

	if (oldInPVS && !inPVS)
	{
		OnLeftPVS(shared_from_this());
		m_LastPVSStateChangeTick = GetWorld()->m_Tick;
	}
	else if (!oldInPVS && inPVS)
	{
		OnEnteredPVS(shared_from_this());
		m_LastPVSStateChangeTick = GetWorld()->m_Tick;
	}
}