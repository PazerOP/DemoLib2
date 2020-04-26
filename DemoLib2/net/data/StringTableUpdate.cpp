#include "StringTableUpdate.hpp"

#include "BitIO/BitIOReader.hpp"
#include "interface/CmdArgs.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/StringTable.hpp"
#include "net/data/UserInfo.hpp"

#include <deque>
#include <iostream>

StringTableUpdate::StringTableUpdate(const StringTable& table, uint_fast16_t entries)
{
	m_EncodeBits = table.GetEncodeBits();
	m_MaxEntries = table.GetMaxEntries();
	m_EntryCount = entries;
	m_Entries.reserve(m_EntryCount);

	m_IsUserDataFixedSize = table.IsUserDataFixedSize();
	if (m_IsUserDataFixedSize)
	{
		m_UserDataSizeBits = table.GetUserDataSizeBits();
		m_UserDataSize = table.GetUserDataSize();
	}
}

StringTableUpdate::StringTableUpdate(uint_fast8_t encodeBits, uint_fast16_t maxEntries, bool isUserDataFixedSize,
	uint_fast8_t userDataSizeBits, uint_fast16_t userDataSize, uint_fast16_t entryCount)
{
	m_EncodeBits = encodeBits;
	m_MaxEntries = maxEntries;
	m_IsUserDataFixedSize = isUserDataFixedSize;
	m_UserDataSizeBits = userDataSizeBits;
	m_UserDataSize = userDataSize;
	m_EntryCount = entryCount;
	m_Entries.reserve(m_EntryCount);
}

void StringTableUpdate::ReadElementInternal(BitIOReader& reader)
{
	m_Entries.clear();

	//if (GetBaseCmdArgs().m_PrintStringTables)
	//	cc::out << cc::fg::yellow << "String table: " << m_Table->GetName() << cc::endl;

	reader.Seek(BitPosition::Zero(), Seek::Start);

	std::deque<std::string> history;

	uint_fast16_t lastEntry = (uint_fast16_t)-1;
	for (uint_fast16_t i = 0; i < m_EntryCount; i++)
	{
		uint_fast16_t entryIndex;
		{
			if (reader.ReadBit())
				entryIndex = lastEntry + 1;
			else
				reader.Read(entryIndex, m_EncodeBits);

			if (entryIndex > m_MaxEntries)
				throw ParseException("Server sent bogus string index for stringtable");

			lastEntry = entryIndex;
		}

		std::string str;
		if (reader.ReadBit())
		{
			if (reader.ReadBit())  // substringCheck
			{
				const auto index = reader.ReadInline<uint_fast8_t>(HISTORY_BITS);
				assert(index <= HISTORY_MAX);
				const auto bytesToCopy = reader.ReadInline<uint_fast8_t>(SUBSTRING_BITS);
				assert(bytesToCopy <= SUBSTRING_MAX);
				str = history[index].substr(0, bytesToCopy);

				str += reader.ReadString();
			}
			else
			{
				str = reader.ReadString();
			}
		}

		BitIOReader userData;
		if (reader.ReadBit())  // User data
		{
			if (m_IsUserDataFixedSize)
			{
				// Length is fixed
				assert(m_UserDataSize > 0);
				userData = reader.TakeSpan(BitPosition::FromBits(m_UserDataSizeBits));
			}
			else
			{
				auto bytes = reader.ReadInline<uint_fast16_t>(MAX_USER_DATA_SIZE_BITS);
				userData = reader.TakeSpan(BitPosition::FromBytes(bytes));
			}
		}

		if (history.size() >= HISTORY_MAX)
			history.pop_front();

		history.push_back(str);
		assert(history.size() <= HISTORY_MAX);

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

// Find the history element with the maximum number of matching characters (up to , starting at the beginning.
StringTableUpdate::HistoryResult StringTableUpdate::FindInHistory(const std::string_view& str, const std::deque<std::string_view>& history)
{
	uint_fast8_t bestIndex = INVALID_HISTORY_INDEX;
	uint_fast8_t bestMatchingChars = 0;

	uint_fast8_t index = 0;
	for (auto& elem : history)
	{
		const uint_fast8_t min = std::min({ uint_fast8_t(str.size()), uint_fast8_t(elem.size()), SUBSTRING_MAX });

		uint_fast8_t matchingChars;
		for (matchingChars = 0; matchingChars < min; matchingChars++)
		{
			if (str[matchingChars] != elem[matchingChars])
				break;
		}

		if (matchingChars == SUBSTRING_MAX)
			return HistoryResult(index, matchingChars);

		if (matchingChars > bestMatchingChars)
		{
			bestMatchingChars = matchingChars;
			bestIndex = index;
		}

		index++;
	}

	return HistoryResult(bestIndex, bestMatchingChars);
}

void StringTableUpdate::WriteElementInternal(BitIOWriter& writer) const
{
	std::deque<std::string_view> history;
	uint_fast16_t lastEntry = (uint_fast16_t)-1;
	for (uint_fast16_t i = 0; i < m_Entries.size(); i++)
	{
		auto& entry = m_Entries[i];

		if (entry.first == lastEntry + 1)
			writer.Write(true);
		else
		{
			writer.Write(false);
			writer.Write(entry.first, m_EncodeBits);
		}

		if (true)// (!entry.second.GetString().empty())
		{
			writer.Write(true);  // Non-empty string

#if true
			if (auto found = FindInHistory(entry.second.GetString(), history); found.index != INVALID_HISTORY_INDEX)
			{
				writer.Write(true);
				writer.Write(found.index, HISTORY_BITS);
				writer.Write(found.chars, SUBSTRING_BITS);

				auto restOfString = std::string_view(entry.second.GetString()).substr(found.chars);
				writer.Write(restOfString);
			}
			else
#endif
			{
				writer.Write(false); // Nothing viable in the history. Just send the full string
				writer.Write(entry.second.GetString());
			}
		}
		else
			writer.Write(false);  // No string content

		if (auto reader = entry.second.GetUserDataReader(); !reader.Length().IsZero())
		{
			writer.Write(true);  // We have user data

			assert(reader.GetLocalPosition().IsZero());
			if (m_IsUserDataFixedSize)
			{
				// Fixed-length userdata
				assert(reader.GetLocalPosition().IsZero());
				writer.Write(reader, BitPosition::FromBits(m_UserDataSizeBits));
			}
			else
			{
				// Variable-length userdata
				assert(reader.Length().Bits() == 0); // Length must be byte-aligned
				writer.Write(reader.Length().TotalBytes(), MAX_USER_DATA_SIZE_BITS);
				writer.Write(reader);
			}
		}
		else
			writer.Write(false);  // We don't have user data

		if (history.size() >= HISTORY_MAX)
			history.pop_front();

		history.push_back(entry.second.GetString());
		assert(history.size() <= HISTORY_MAX);

		lastEntry = i;
	}
}

IStreamElement* StringTableUpdate::CreateNewInstance() const
{
	return new StringTableUpdate(m_EncodeBits, m_MaxEntries, m_IsUserDataFixedSize,
		m_UserDataSizeBits, m_UserDataSize, m_EntryCount);
}

void StringTableUpdate::ApplyUpdate(StringTable& table) const
{
	for (const auto& entry : m_Entries)
		table.Get(entry.first) = entry.second;

	if (!m_Entries.empty())
		table.OnUpdated();
}