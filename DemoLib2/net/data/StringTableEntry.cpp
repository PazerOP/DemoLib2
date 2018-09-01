#include "StringTableEntry.hpp"

StringTableEntry::StringTableEntry(std::string&& string) :
	m_String(string)
{
}
StringTableEntry::StringTableEntry(std::string&& string, BitIOReader&& reader) :
	m_String(string), m_UserData(reader)
{
}
StringTableEntry::StringTableEntry(std::string&& string, BitIOWriter&& writer) :
	m_String(string), m_UserData(writer)
{
}

#if 0
StringTableEntry& StringTableEntry::operator=(const StringTableEntry& other)
{
	m_String = other.m_String;

	if (std::holds_alternative<BitIOReader>(other.m_UserData))
		m_UserData = std::get<BitIOReader>(other.m_UserData);
	else
		m_UserData = std::get<BitIOWriter>(other.m_UserData);

	return *this;
}
#endif

BitIOWriter& StringTableEntry::GetUserDataWriter()
{
	if (std::holds_alternative<BitIOReader>(m_UserData))
	{
		BitIOWriter writer(true);
		BitIOReader reader = std::get<BitIOReader>(m_UserData);
		reader.Seek(BitPosition::Zero(), Seek::Start);
		writer.Write(reader);
		m_UserData = std::move(writer);
	}

	return std::get<BitIOWriter>(m_UserData);
}

const BitIOReader& StringTableEntry::GetUserDataReader() const
{
	if (std::holds_alternative<BitIOWriter>(m_UserData))
		return std::get<BitIOWriter>(m_UserData);
	else
		return std::get<BitIOReader>(m_UserData);
}

void StringTableEntry::ClearUserData()
{
	m_UserData = BitIOReader();
}

void StringTableEntry::SetUserData(const BitIOReader& reader)
{
	m_UserData = reader;
}

void StringTableEntry::SetUserData(const BitIOWriter& writer)
{
	m_UserData = writer;
}

void StringTableEntry::Clear()
{
	m_String.clear();
	ClearUserData();
}
