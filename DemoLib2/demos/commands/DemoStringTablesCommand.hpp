#pragma once

#include "BitIO/BitIOReader.hpp"
#include "demos/commands/TimestampedDemoCommand.hpp"

#include <optional>

class StringTableEntry;

class DemoStringTablesCommand : public TimestampedDemoCommand
{
public:
	DemoCommandType GetType() const override { return DemoCommandType::dem_stringtables; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoStringTablesCommand(); }

	void ApplyWorldState(WorldState& world) const override;

private:
	static constexpr uint_fast16_t ENTRY_MAX_LENGTH = 4096;
	static constexpr uint_fast16_t TABLENAME_MAX_LENGTH = 256;

	static void DecodeStringTable(BitIOReader& reader, WorldState& world);
	static void DecodeStringTableEntry(BitIOReader& reader, WorldState& world, StringTableEntry& entry);

	BitIOReader m_Data;
};