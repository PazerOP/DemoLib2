#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/data/GameEvent.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetGameEventMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_GAMEEVENT; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetGameEventMessage(); }

private:
	static constexpr uint_fast8_t EVENT_LENGTH_BITS = 11;
	static constexpr uint_fast8_t EVENT_ID_BITS = 9;
	static constexpr uint_fast16_t MAX_EVENT_BYTES = 1024;
	static constexpr uint_fast8_t MAX_EVENT_NAME_LENGTH = 32;

	BitIOReader m_Data;
};