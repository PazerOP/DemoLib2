#include "DemoStringTablesCommand.hpp"

#include "BitIO/BitIOWriter.hpp"
#include "net/data/StringTable.hpp"
#include "net/worldstate/WorldState.hpp"

void DemoStringTablesCommand::ReadElementInternal(BitIOReader& reader)
{
	TimestampedDemoCommand::ReadElementInternal(reader);

	auto length = reader.ReadInline<uint32_t>("DemoStringTablesCommand length");
	auto data = reader.TakeSpan(BitPosition::FromBytes(length));

	DecodeStringTables(data);

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << STR_FILEBITS(reader) << cc::fg::green << ' ' << GetType() << ": length " << length << cc::endl;
}

void DemoStringTablesCommand::WriteElementInternal(BitIOWriter& writer) const
{
	TimestampedDemoCommand::WriteElementInternal(writer);

	const auto dataPosition = writer.GetPosition();
	writer.Write<uint32_t>(0);

	const auto startPos = writer.GetPosition();

	assert(m_Tables.size() <= std::numeric_limits<uint8_t>::max());
	writer.Write<uint8_t>(uint8_t(m_Tables.size()));
	for (auto& table : m_Tables)
	{
		writer.Write(table.first, TABLENAME_MAX_LENGTH);

		auto& entries = table.second;
		assert(entries.size() <= std::numeric_limits<uint16_t>::max());
		writer.Write<uint16_t>(uint16_t(entries.size()));
		for (auto& entry : entries)
		{
			writer.Write(entry.GetString(), ENTRY_MAX_LENGTH);

			auto userdata = entry.GetUserDataReader();
			assert(userdata.GetLocalPosition().IsZero());
			if (!userdata.Length().IsZero())
			{
				writer.Write(true);
				assert(userdata.Length().TotalBytes() <= std::numeric_limits<uint16_t>::max());
				writer.Write<uint16_t>(uint16_t(userdata.Length().TotalBytes()));
				writer.Write(userdata);

				// Pad with zeros so we always have full bytes
				if (userdata.Length().Bits())
					writer.Write(0, 8 - userdata.Length().Bits());
			}
			else
				writer.Write(false);
		}

		writer.Write(false); // Not exactly sure what "client side" stringtable entries are for, but we probably don't need to worry about them?
	}

	writer.PadToByte();

	const auto endPos = writer.GetPosition();
	assert((endPos - startPos).IsByteAligned());
	writer.Seek(dataPosition, Seek::Set);

	assert((endPos - startPos).TotalBytes() <= std::numeric_limits<uint32_t>::max());
	writer.Write<uint32_t>(uint32_t((endPos - startPos).TotalBytes()));
	writer.Seek(endPos, Seek::Set);
}

void DemoStringTablesCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);

	for (auto& tableData : m_Tables)
	{
		const auto& table = world.FindStringTable(tableData.first);
		assert(table);

		// We have solid evidence now that dem_stringtables commands clear any pre-existing
		// strings in the target stringtable.
		for (auto& entry : *table)
			entry.Clear();

		auto& entries = tableData.second;
		for (uint_fast16_t i = 0; i < entries.size(); i++)
			table->Get(i) = entries[i];
	}
}

void DemoStringTablesCommand::DecodeStringTables(BitIOReader& reader)
{
	m_Tables.clear();

	const auto tableCount = reader.ReadInline<uint8_t>("String table count");
	for (uint_fast8_t t = 0; t < tableCount; t++)
	{
		auto& tableEntries = m_Tables[reader.ReadString(TABLENAME_MAX_LENGTH)];

		auto entries = reader.ReadInline<uint16_t>();
		for (uint_fast16_t i = 0; i < entries; i++)
			DecodeStringTableEntry(reader, tableEntries);

		if (reader.ReadBit())
		{
			// Clientside entries
			auto clientsideEntries = reader.ReadInline<uint16_t>();
			for (uint_fast16_t i = 0; i < clientsideEntries; i++)
				DecodeStringTableEntry(reader, tableEntries);
		}
	}
}

void DemoStringTablesCommand::DecodeStringTableEntry(BitIOReader& reader, std::vector<StringTableEntry>& entries)
{
	auto& entry = entries.emplace_back();
	entry.GetString() = reader.ReadString(ENTRY_MAX_LENGTH);

	if (reader.ReadBit())
	{
		auto dataLength = reader.ReadInline<uint16_t>();
		entry.SetUserData(reader.TakeSpan(BitPosition::FromBytes(dataLength)));
	}
}
