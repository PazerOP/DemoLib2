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
	//if (maxEntries > 0)
	//	return std::min<uint_fast8_t>(Log2(maxEntries), 1);
	//else
	//	return 0;

	return Log2(maxEntries);
}

bool StringTable::IsUserDataFixedSize() const
{
	assert(m_UserDataSize.has_value() == m_UserDataSizeBits.has_value());
	return m_UserDataSize.has_value();
}

uint_fast16_t StringTable::FindLowestUnused() const
{
	for (uint_fast16_t i = 0; i < m_MaxEntries; i++)
	{
		if (m_Entries[i].IsEmpty())
			return i;
	}

	return uint_fast16_t(-1);
}