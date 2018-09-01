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

	const auto changedEntries = m_Update ? m_Update->GetEntryCount() : m_ChangedEntries;

	assert(changedEntries >= 1);
	writer.Write(changedEntries > 1);
	if (changedEntries > 1)
		writer.Write(changedEntries, CHANGED_ENTRIES_BITS);

	if (m_Update)
	{
		BitIOWriter tempWriter(true);
		m_Update->WriteElement(tempWriter);

		tempWriter.SeekBits(0, Seek::Start);
		writer.Write(tempWriter.Length().TotalBits(), DATA_LENGTH_BITS);
		writer.Write(tempWriter);
	}
	else
	{
		writer.Write(m_Data.Length().TotalBits(), DATA_LENGTH_BITS);
		auto clone = m_Data;
		clone.Seek(BitPosition::Zero(), Seek::Start);
		writer.Write(clone);
	}
}

NetUpdateStringTableMessage::NetUpdateStringTableMessage(const StringTable& table, KnownStringTable index) :
	m_TableID(uint_fast8_t(index))
{
	m_Update.emplace(table, 0);
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
	if (!m_Update)
	{
		StringTableUpdate update(*stringTable, m_ChangedEntries);

		auto clone = m_Data;
		clone.Seek(BitPosition::Zero(), Seek::Start);
		update.ReadElement(clone);

		update.ApplyUpdate(*stringTable);
	}
	else
	{
		m_Update->ApplyUpdate(*stringTable);
	}

	world.m_Events.PostStringTableUpdate(stringTable);
}