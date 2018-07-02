#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/data/BaselineIndex.hpp"
#include "net/netmessages/INetMessage.hpp"

#include <optional>

class Entity;

class NetPacketEntitiesMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_PACKETENTITIES; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetPacketEntitiesMessage(); }

private:
	static constexpr uint_fast8_t DELTA_INDEX_BITS = 32;
	static constexpr uint_fast8_t DELTA_SIZE_BITS = 20;

	std::shared_ptr<Entity> ReadEnterPVS(WorldState& world, BitIOReader& reader, uint_fast16_t entityIndex) const;
	void ReadLeavePVS(WorldState& world, uint_fast16_t entIndex, bool bDelete) const;

	uint_fast32_t m_MaxEntries;
	uint_fast32_t m_UpdatedEntries;
	bool m_IsDelta;
	bool m_UpdateBaseline;
	BaselineIndex m_Baseline;
	std::optional<uint_fast32_t> m_DeltaFrom;

	BitIOReader m_Data;
};