#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetUpdateStringTableMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_UPDATESTRINGTABLE; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetUpdateStringTableMessage(); }

private:
	static constexpr uint_fast8_t DATA_LENGTH_BITS = 20;
	static constexpr uint_fast8_t CHANGED_ENTRIES_BITS = 16;

	uint_fast8_t m_TableID;
	uint_fast16_t m_ChangedEntries;

	BitIOReader m_Data;
};