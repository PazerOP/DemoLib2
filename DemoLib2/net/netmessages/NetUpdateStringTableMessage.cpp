#include "NetUpdateStringTableMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/data/StringTableUpdate.hpp"
#include "net/worldstate/WorldState.hpp"

#include <inttypes.h>
#include <iostream>

void NetUpdateStringTableMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read<uint_fast8_t>(m_TableID, MAX_STRINGTABLE_BITS);

	bool multipleChanged = reader.ReadBit();
	if (!multipleChanged)
		m_ChangedEntries = 1;
	else
		reader.Read(m_ChangedEntries, CHANGED_ENTRIES_BITS);

	auto bitCount = reader.ReadInline<uint_fast32_t>(DATA_LENGTH_BITS);
	m_Data = reader.TakeSpan(BitPosition::FromBits(bitCount));
}
void NetUpdateStringTableMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_TableID, MAX_STRINGTABLE_BITS);

	assert(m_ChangedEntries >= 1);
	writer.Write(m_ChangedEntries > 1);
	if (m_ChangedEntries > 1)
	{
		writer.Write(m_ChangedEntries, CHANGED_ENTRIES_BITS);
	}

	writer.Write(m_Data.Length().TotalBits(), DATA_LENGTH_BITS);
	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone);
}

void NetUpdateStringTableMessage::GetDescription(std::ostream& description) const
{
	description << "svc_UpdateStringTable: table " << +m_TableID <<
		", changed " << m_ChangedEntries <<
		", bytes " << m_Data.Length().TotalBytes();
}
void NetUpdateStringTableMessage::ApplyWorldState(WorldState& world) const
{
	auto stringTable = world.m_StringTables[m_TableID];

	// Parse and apply new table update
	{
		StringTableUpdate update(stringTable, m_ChangedEntries);

		auto clone = m_Data;
		clone.Seek(BitPosition::Zero(), Seek::Start);
		update.ReadElement(clone);

		update.ApplyUpdate();
		world.m_Events.PostStringTableUpdate(stringTable);
	}
}