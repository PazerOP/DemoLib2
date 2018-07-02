#include "DemoStringTablesCommand.hpp"

#include "BitIO/BitIOWriter.hpp"
#include "net/data/StringTable.hpp"
#include "net/worldstate/WorldState.hpp"

void DemoStringTablesCommand::ReadElementInternal(BitIOReader& reader)
{
	TimestampedDemoCommand::ReadElementInternal(reader);

	// We don't need to decode string tables for now
	auto length = reader.ReadInline<uint32_t>("DemoStringTablesCommand length");
	m_Data = reader.TakeSpan(BitPosition::FromBytes(length));

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << STR_FILEBITS(reader) << cc::fg::green << ' ' << GetType() << ": length " << length << cc::endl;
}

void DemoStringTablesCommand::WriteElementInternal(BitIOWriter& writer) const
{
	TimestampedDemoCommand::WriteElementInternal(writer);

	assert(m_Data.Length().IsByteAligned());
	writer.Write(m_Data.Length().Bytes(), 32);

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone);
}

void DemoStringTablesCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);

	auto clone = m_Data;
	assert(!clone.GetLocalPosition());

	const auto count = clone.ReadInline<uint8_t>();
	for (uint_fast8_t i = 0; i < count; i++)
		DecodeStringTable(clone, world);
}

void DemoStringTablesCommand::DecodeStringTable(BitIOReader& reader, WorldState& world)
{
	auto tableName = reader.ReadString(TABLENAME_MAX_LENGTH);

	std::shared_ptr<StringTable> table = world.FindStringTable(tableName);
	assert(table);

	auto entries = reader.ReadInline<uint16_t>();
	for (uint_fast16_t i = 0; i < entries; i++)
	{
		DecodeStringTableEntry(reader, world, table->Get(i));
	}

	if (reader.ReadBit())
	{
		// Clientside entries
		auto clientsideEntries = reader.ReadInline<uint16_t>();
		for (uint_fast16_t i = 0; i < clientsideEntries; i++)
		{
			DecodeStringTableEntry(reader, world, table->Get(entries + i));
		}
	}
}

void DemoStringTablesCommand::DecodeStringTableEntry(BitIOReader& reader, WorldState& world, StringTableEntry& entry)
{
	entry.GetString() = reader.ReadString(ENTRY_MAX_LENGTH);

	BitIOReader data;
	if (reader.ReadBit())
	{
		auto dataLength = reader.ReadInline<uint16_t>();
		entry.SetUserData(reader.TakeSpan(BitPosition::FromBytes(dataLength)));
	}
	else
		entry.ClearUserData();
}
