#include "DemoDataTablesCommand.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "net/data/SendTable.hpp"
#include "net/data/ServerClass.hpp"
#include "net/worldstate/WorldState.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>

void DemoDataTablesCommand::ReadElementInternal(BitIOReader& _fullReader)
{
	TimestampedDemoCommand::ReadElementInternal(_fullReader);

	BitIOReader reader;
	{
		const auto bytes = _fullReader.ReadInline<uint32_t>("DemoDataTablesCommand length");
		reader = _fullReader.TakeSpan(BitPosition::FromBytes(bytes));
	}

	while (reader.ReadBit("DemoDataTablesCommand: Another SendTable?"))
	{
		auto& table = m_SendTables.emplace_back(std::make_shared<SendTable>());
		table->ReadElement(reader);
	}

	// Link referenced DataTables
	for (auto& sendTable : m_SendTables)
	{
		for (auto& propDef : sendTable->GetProperties())
		{
			if (propDef->GetType() != SendPropType::Datatable)
				continue;

			const auto& foundTable = std::find_if(m_SendTables.begin(), m_SendTables.end(),
				[&propDef](const auto& a) { return !strcmp(propDef->GetExcludeName().c_str(), a->GetName().c_str()); });

			if (foundTable == m_SendTables.end())
				throw std::runtime_error("malformed demo");

			// Dumb hack
			const_cast<SendPropDefinition&>(*propDef).TryConnectDataTable(*foundTable);
		}
	}

	for (auto& sendTable : m_SendTables)
		sendTable->InitPostDTConnect();

	const auto serverClasses = reader.ReadInline<uint_fast16_t>(16);
	assert(serverClasses > 0);

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << '\t' << serverClasses << " server classes:" << std::endl;

	for (uint_fast16_t i = 0; i < serverClasses; i++)
	{
		const auto id = reader.ReadInline<uint16_t>("ID");
		auto className = reader.ReadString("Classname");
		const auto datatableName = reader.ReadString("Datatable Name");

		auto& serverClass = m_ServerClasses.emplace_back(std::make_shared<ServerClass>(id, std::move(className), FindSendTable(datatableName)));

		if (GetBaseCmdArgs().m_PrintDemo)
			cc::out << "\t\t[" << serverClass->GetID() << "] " << serverClass->GetClassname() << ": " << serverClass->GetSendTable()->GetName() << cc::endl;
	}

	if (reader.Remaining().Bytes() > 0)
		throw std::runtime_error("malformed demo");
	else if (reader.Remaining().Bits() > 0)
		cc::out << cc::fg::magenta << "Finished " << GetType() << " command with " << +reader.Remaining().Bits() << " bits remaining" << cc::endl;
}

void DemoDataTablesCommand::WriteElementInternal(BitIOWriter& writer) const
{
	TimestampedDemoCommand::WriteElementInternal(writer);

	BitIOWriter tempWriter(true);

	for (const auto& st : m_SendTables)
	{
		tempWriter.Write(true); // Upcoming sendtable
		st->WriteElement(tempWriter);
	}

	tempWriter.Write(false);  // Done with sendtables

	tempWriter.Write(m_ServerClasses.size(), 16);

	for (const auto& sc : m_ServerClasses)
	{
		tempWriter.Write<uint16_t>(sc->GetID());
		tempWriter.Write(sc->GetClassname());
		tempWriter.Write(sc->GetSendTable()->GetName());
	}

	// Pad to bytes
	if (tempWriter.Length().Bits())
		tempWriter.Write<uint_fast8_t>(0, 8 - tempWriter.Length().Bits());

	tempWriter.Seek(BitPosition::Zero(), Seek::Start);
	assert(tempWriter.Length().IsByteAligned());
	assert(tempWriter.Length().TotalBytes() <= std::numeric_limits<uint32_t>::max());
	writer.Write<uint32_t>((uint32_t)tempWriter.Length().TotalBytes());
	writer.Write(tempWriter);
}

void DemoDataTablesCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);

	// SendTables
	{
		world.m_SendTables.clear();
		for (const auto& st : m_SendTables)
			world.m_SendTables[st->GetName()] = st;
		world.m_Events.PostSendTableListLoad();
	}

	// ServerClasses
	{
		std::vector<std::shared_ptr<const ServerClass>> clone;
		clone.reserve(m_ServerClasses.size());
		for (const auto& sc : m_ServerClasses)
			clone.push_back(sc);

		world.m_Events.PreServerClassListLoad(clone);
		world.m_ServerClasses = std::move(clone);
		world.m_Events.PostServerClassListLoad();
	}
}

std::shared_ptr<const SendTable> DemoDataTablesCommand::FindSendTable(const std::string_view& name) const
{
	for (const auto& st : m_SendTables)
	{
		if (st->GetName() == name)
			return st;
	}

	assert(!"Unable to find SendTable with the given name");
	return nullptr;
}
