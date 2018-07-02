#include "StringTable.hpp"

StringTable::StringTable(const std::weak_ptr<WorldState>& world, std::string&& tableName, uint_fast16_t maxEntries,
	const std::optional<uint_fast16_t>& userDataSize, const std::optional<uint_fast8_t>& userDataSizeBits) :
	m_World(world),
	m_Name(tableName),
	m_UserDataSize(userDataSize), m_UserDataSizeBits(userDataSizeBits)
{
	m_World = world;
	m_Name = tableName;
	m_MaxEntries = maxEntries;
	m_UserDataSize = userDataSize;
	m_UserDataSizeBits = userDataSizeBits;

	m_Entries.reset(new StringTableEntry[m_MaxEntries]);
}

uint_fast8_t StringTable::GetEncodeBits(uint_fast16_t maxEntries)
{
	return Log2(maxEntries);
}

bool StringTable::IsUserDataFixedSize() const
{
	assert(m_UserDataSize.has_value() == m_UserDataSizeBits.has_value());
	return m_UserDataSize.has_value();
}