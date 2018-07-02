#pragma once

#include "BitIO/IStreamElement.hpp"

#include "net/data/StringTableEntry.hpp"

#include <memory>
#include <unordered_map>

class StringTable;

class StringTableUpdate final : public IStreamElement
{
public:
	StringTableUpdate(const std::shared_ptr<StringTable>& table, uint_fast16_t entryCount);

	void ApplyUpdate();

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;

	IStreamElement* CreateNewInstance() const override { return new StringTableUpdate(m_Table, m_EntryCount); }

private:
	static constexpr uint_fast8_t HISTORY_BITS = 5;
	static constexpr uint_fast8_t SUBSTRING_BITS = 5;

	static constexpr uint_fast8_t MAX_USER_DATA_SIZE_BITS = 14;

	uint_fast16_t m_EntryCount;
	std::vector<std::pair<uint_fast16_t, StringTableEntry>> m_Entries;
	std::shared_ptr<StringTable> m_Table;
};