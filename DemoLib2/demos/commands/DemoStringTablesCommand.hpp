#pragma once

#include "BitIO/BitIOReader.hpp"
#include "demos/commands/TimestampedDemoCommand.hpp"
#include "net/data/StringTableEntry.hpp"

#include <map>
#include <optional>

class DemoStringTablesCommand : public TimestampedDemoCommand
{
public:
	DemoCommandType GetType() const override { return DemoCommandType::dem_stringtables; }

	auto& GetTables() { return m_Tables; }
	auto& GetTables() const { return m_Tables; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoStringTablesCommand(); }

	void ApplyWorldState(WorldState& world) const override;

private:
	static constexpr uint_fast16_t ENTRY_MAX_LENGTH = 4096;
	static constexpr uint_fast16_t TABLENAME_MAX_LENGTH = 256;

	void DecodeStringTables(BitIOReader& reader);
	void DecodeStringTableEntry(BitIOReader& reader, std::vector<StringTableEntry>& entries);

	std::map<std::string, std::vector<StringTableEntry>> m_Tables;
};