#pragma once

#include "BitIO/IStreamElement.hpp"

#include "net/data/StringTableEntry.hpp"

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

class StringTable;

class StringTableUpdate final : public IStreamElement
{
public:
	StringTableUpdate(const StringTable& table, uint_fast16_t entryCount);
	StringTableUpdate(uint_fast8_t encodeBits, uint_fast16_t maxEntries, bool isUserDataFixedSize,
		uint_fast8_t userDataSizeBits, uint_fast16_t userDataSize, uint_fast16_t entryCount);

	void ApplyUpdate(StringTable& table) const;

	size_t GetEntryCount() const { return m_Entries.size(); }
	auto& GetEntries() { return m_Entries; }
	auto GetMaxEntries() const { return m_MaxEntries; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override;

private:
	static constexpr uint_fast8_t INVALID_HISTORY_INDEX = (uint_fast8_t)-1;

	struct HistoryResult
	{
		constexpr HistoryResult(uint_fast8_t index, uint_fast8_t chars) : index(index), chars(chars) {}

		uint_fast8_t index;
		uint_fast8_t chars;
	};

	static HistoryResult FindInHistory(const std::string_view& str, const std::deque<std::string_view>& history);

	static constexpr uint_fast8_t HISTORY_BITS = 5;
	static constexpr uint_fast8_t HISTORY_MAX = 32;
	static constexpr uint_fast8_t SUBSTRING_BITS = 5;
	static constexpr uint_fast8_t SUBSTRING_MAX = 31;

	static constexpr uint_fast8_t MAX_USER_DATA_SIZE_BITS = 14;

	uint_fast8_t m_EncodeBits;
	uint_fast16_t m_MaxEntries;

	bool m_IsUserDataFixedSize;
	uint_fast8_t m_UserDataSizeBits;
	uint_fast16_t m_UserDataSize;

	uint_fast16_t m_EntryCount;
	std::vector<std::pair<uint_fast16_t, StringTableEntry>> m_Entries;
};