#pragma once

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"

#include <variant>

class StringTableEntry
{
public:
	StringTableEntry() = default;
	StringTableEntry(std::string&& string);
	StringTableEntry(std::string&& string, BitIOReader&& reader);
	StringTableEntry(std::string&& string, BitIOWriter&& writer);

	inline const bool IsEmpty() const { return m_String.empty() && !GetUserDataReader().Length(); }

	std::string& GetString() { return m_String; }
	const std::string& GetString() const { return m_String; }

	BitIOWriter& GetUserDataWriter();
	const BitIOReader& GetUserDataReader() const;

	void ClearUserData();
	void SetUserData(const BitIOReader& reader);
	void SetUserData(const BitIOWriter& writer);

	//StringTableEntry& operator=(const StringTableEntry& other);

private:
	std::string m_String;
	std::variant<BitIOReader, BitIOWriter> m_UserData;
};