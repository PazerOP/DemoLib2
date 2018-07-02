#pragma once

#include "misc/Event.hpp"
#include "net/entities/IBaseEntity.hpp"

#include <vector>

class Entity final : public IBaseEntity, public std::enable_shared_from_this<Entity>
{
public:
	Entity(const std::weak_ptr<WorldState>& world, const std::shared_ptr<const ServerClass>& serverClass, const std::shared_ptr<const SendTable>& networkTable, uint_fast16_t entityIndex, uint_fast16_t serialNumber);

	auto GetIndex() const { return m_Index; }
	auto GetSerialNumber() const { return m_SerialNumber; }

	bool IsInPVS() const { return m_InPVS; }
	void SetInPVS(bool inPVS);

	bool IsTempEntity() const override { return false; }

	auto GetCreatedTick() const { return m_CreatedTick; }
	auto GetLastPVSStateChangeTick() const { return m_LastPVSStateChangeTick; }

private:
	uint_fast16_t m_Index;
	uint_fast16_t m_SerialNumber;

	uint_fast32_t m_CreatedTick;

	bool m_InPVS;
	uint_fast32_t m_LastPVSStateChangeTick;

public:
	Event<const std::shared_ptr<Entity>&> OnEnteredPVS;
	Event<const std::shared_ptr<Entity>&> OnLeftPVS;
	Event<const std::shared_ptr<Entity>&> OnDeleted;
};