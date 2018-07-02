#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"

#include <memory>
#include <optional>
#include <string>

class StringTable;
class StringTableUpdate;

class NetCreateStringTableMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_CREATESTRINGTABLE; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetCreateStringTableMessage(); }

private:
	static constexpr uint_fast8_t USER_DATA_SIZE_BITS = 12;
	static constexpr uint_fast8_t USER_DATA_SIZE_BITS_BITS = 4;

	BitIOWriter Compress() const;

	std::string m_TableName;

	uint_fast16_t m_Entries;
	uint_fast16_t m_MaxEntries;

	std::optional<uint_fast16_t> m_UserDataSize;
	std::optional<uint_fast8_t> m_UserDataSizeBits;

	bool m_IsFilenames;
	bool m_WasDataCompressed;
	BitIOReader m_RawData;
	BitIOReader m_DecompressedData;
};