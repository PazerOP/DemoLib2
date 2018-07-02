#pragma once

#include "misc/Event.hpp"
#include "net/data/StringTableEntry.hpp"

#include <cassert>

class WorldState;

class StringTable final : public std::enable_shared_from_this<StringTable>
{
public:
	StringTable(const std::weak_ptr<WorldState>& world, std::string&& tableName, uint_fast16_t maxEntries,
		const std::optional<uint_fast16_t>& userDataSize, const std::optional<uint_fast8_t>& userDataSizeBits);

	std::shared_ptr<WorldState> GetWorld() { return m_World.lock(); }
	std::shared_ptr<const WorldState> GetWorld() const { return m_World.lock(); }

	const std::string& GetName() const { return m_Name; }
	uint_fast8_t GetEncodeBits() const { return StringTable::GetEncodeBits(m_MaxEntries); }
	static uint_fast8_t GetEncodeBits(uint_fast16_t maxEntries);
	uint_fast16_t GetMaxEntries() const { return m_MaxEntries; }

	StringTableEntry& Get(uint_fast16_t i) { assert(i < m_MaxEntries); return m_Entries[i]; }
	const StringTableEntry& Get(uint_fast16_t i) const { assert(i < m_MaxEntries); return m_Entries[i]; }
	StringTableEntry& operator[](uint_fast16_t i) { return Get(i); }
	const StringTableEntry& operator[](uint_fast16_t i) const { return Get(i); }

	StringTableEntry* GetEntries() { return m_Entries.get(); }
	const StringTableEntry* GetEntries() const { return m_Entries.get(); }

	StringTableEntry* begin() { return m_Entries.get(); }
	const StringTableEntry* begin() const { return m_Entries.get(); }
	StringTableEntry* end() { return m_Entries.get() + m_MaxEntries; }
	const StringTableEntry* end() const { return m_Entries.get() + m_MaxEntries; }

	bool IsUserDataFixedSize() const;

	uint_fast16_t GetUserDataSize() const { return m_UserDataSize.value(); }
	uint_fast8_t GetUserDataSizeBits() const { return m_UserDataSizeBits.value(); }

private:
	std::string m_Name;
	uint_fast16_t m_MaxEntries;

	std::unique_ptr<StringTableEntry[]> m_Entries;

	std::optional<uint_fast16_t> m_UserDataSize;
	std::optional<uint_fast8_t> m_UserDataSizeBits;

	std::weak_ptr<WorldState> m_World;

public:
	Event<const std::shared_ptr<StringTable>&> OnUpdate;
};