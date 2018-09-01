#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"
#include "net/data/StringTableUpdate.hpp"

enum class KnownStringTable : uint8_t;

class NetUpdateStringTableMessage : public INetMessage
{
public:
	NetUpdateStringTableMessage() = default;
	NetUpdateStringTableMessage(const StringTable& table, KnownStringTable index);

	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_UPDATESTRINGTABLE; }

	auto GetTableID() const { return m_TableID; }

	auto& GetUpdate() { return m_Update.value(); }
	auto& GetUpdate() const { return m_Update.value(); }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetUpdateStringTableMessage(); }

private:
	static constexpr uint_fast8_t DATA_LENGTH_BITS = 20;
	static constexpr uint_fast8_t CHANGED_ENTRIES_BITS = 16;

	std::optional<StringTableUpdate> m_Update;

	uint_fast8_t m_TableID;
	uint_fast16_t m_ChangedEntries;

	BitIOReader m_Data;
};