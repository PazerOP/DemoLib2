#pragma once

#include "net/data/GameEventDeclaration.hpp"
#include "net/netmessages/INetMessage.hpp"

#include <vector>

class NetGameEventListMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_GAMEEVENTLIST; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetGameEventListMessage(); }

private:
	static constexpr uint_fast8_t BIT_COUNT_BITS = 20;
	static constexpr uint_fast8_t EVENT_DATA_TYPE_BITS = 3;

	std::vector<GameEventDeclaration> m_Events;
	uint32_t m_BitCount;  // Only for the description
};