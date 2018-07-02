#include "StringTableUpdate.hpp"

#include "BitIO/BitIOReader.hpp"
#include "interface/CmdArgs.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/StringTable.hpp"
#include "net/data/UserInfo.hpp"

#include <deque>
#include <iostream>

StringTableUpdate::StringTableUpdate(const std::shared_ptr<StringTable>& table, uint_fast16_t entries)
{
	m_Table = table;
	m_EntryCount = entries;
	m_Entries.reserve(m_EntryCount);
}

void StringTableUpdate::ReadElementInternal(BitIOReader& reader)
{
	m_Entries.clear();

	if (GetBaseCmdArgs().m_PrintStringTables)
		cc::out << cc::fg::yellow << "String table: " << m_Table->GetName() << cc::endl;

	const auto encodeBits = m_Table->GetEncodeBits();

	reader.Seek(BitPosition::Zero(), Seek::Start);

	std::deque<std::string> history;

	uint_fast16_t lastEntry = (uint_fast16_t)-1;
	for (uint_fast16_t i = 0; i < m_EntryCount; i++)
	{
		std::string str;
		BitIOReader userData;

		uint_fast16_t entryIndex;
		{
			if (reader.ReadBit())
				entryIndex = lastEntry + 1;
			else
				reader.Read(entryIndex, encodeBits);

			if (entryIndex > m_Table->GetMaxEntries())
				throw ParseException("Server sent bogus string index for stringtable");

			lastEntry = entryIndex;
		}

		if (reader.ReadBit())
		{
			if (reader.ReadBit())  // substringCheck
			{
				const auto index = reader.ReadInline<uint_fast8_t>(HISTORY_BITS);
				const auto bytesToCopy = reader.ReadInline<uint_fast8_t>(SUBSTRING_BITS);
				str = history[index].substr(0, bytesToCopy);

				str += reader.ReadString();
			}
			else
			{
				str = reader.ReadString();
			}
		}

		if (reader.ReadBit())  // User data
		{
			if (m_Table->IsUserDataFixedSize())
			{
				// Length is fixed
				assert(m_Table->GetUserDataSize() > 0);
				userData = reader.TakeSpan(BitPosition::FromBits(m_Table->GetUserDataSizeBits()));
			}
			else
			{
				auto bytes = reader.ReadInline<uint_fast16_t>(MAX_USER_DATA_SIZE_BITS);
				userData = reader.TakeSpan(BitPosition::FromBytes(bytes));
			}
		}

		if (history.size() > 31)
			history.pop_front();

		history.push_back(str);

		if (GetBaseCmdArgs().m_PrintStringTables)
		{
			cc::out << cc::fg::yellow << cc::bold << "\tentry " << entryIndex << ": \"" << str << '"';

			if (!userData.Length().IsZero())
				cc::out << " (userData: " << userData.Length() << ')';

			cc::out << cc::endl;
		}

		// Add the entry
		m_Entries.emplace_back(entryIndex, StringTableEntry(std::move(str), std::move(userData)));
	}
}
void StringTableUpdate::WriteElementInternal(BitIOWriter& writer) const
{
	throw NotImplementedException();
}

void StringTableUpdate::ApplyUpdate()
{
	for (const auto& entry : m_Entries)
		m_Table->Get(entry.first) = entry.second;

	if (!m_Entries.empty())
		m_Table->OnUpdate(m_Table);
}