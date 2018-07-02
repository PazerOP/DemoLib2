#include "DemoFile.hpp"

#include "BitIO/BitIOWriter.hpp"
#include "demos/DemoCommandType.hpp"
#include "demos/DemoHeader.hpp"
#include "demos/commands/DemoConsoleCommand.hpp"
#include "demos/commands/DemoDataTablesCommand.hpp"
#include "demos/commands/DemoPacketCommand.hpp"
#include "demos/commands/DemoSignonCommand.hpp"
#include "demos/commands/DemoStringTablesCommand.hpp"
#include "demos/commands/DemoSyncTickCommand.hpp"
#include "demos/commands/DemoUserCommand.hpp"

#include <iostream>

std::shared_ptr<IDemoCommand> DemoFile::CreateDemoCommand(DemoCommandType type)
{
	switch (type)
	{
		case DemoCommandType::dem_signon:
			return std::make_shared<DemoSignonCommand>();

		case DemoCommandType::dem_datatables:
			return std::make_shared<DemoDataTablesCommand>();

		case DemoCommandType::dem_packet:
			return std::make_shared<DemoPacketCommand>();

		case DemoCommandType::dem_synctick:
			return std::make_shared<DemoSyncTickCommand>();

		case DemoCommandType::dem_consolecmd:
			return std::make_shared<DemoConsoleCommand>();

		case DemoCommandType::dem_usercmd:
			return std::make_shared<DemoUserCommand>();

		case DemoCommandType::dem_stringtables:
			return std::make_shared<DemoStringTablesCommand>();

		case DemoCommandType::dem_stop:
			return nullptr;

		default:
			throw std::logic_error("not implemented");
	};
}

std::shared_ptr<IDemoCommand> DemoFile::ReadCommand(BitIOReader& reader)
{
	const auto cmdType = reader.ReadInline<DemoCommandType>();
	if (cmdType == DemoCommandType::dem_stop)
		return nullptr;

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << STR_FILEBITS(reader) << cc::fg::green << cc::bold << "Beginning read of " << cmdType << "..." << cc::endl;

	auto cmd = CreateDemoCommand(cmdType);
	cmd->ReadElement(reader);
	return cmd;
}

DemoFile::DemoCmdVec& DemoFile::ReadCommands(BitIOReader& reader, DemoCmdVec& commands)
{
	while (auto cmd = ReadCommand(reader))
		commands.emplace_back(std::move(cmd));

	return commands;
}

void DemoFile::ReadElementInternal(BitIOReader& reader)
{
	m_Header.ReadElement(reader);
	cc::out << cc::fg::cyan << "Demo header: " << m_Header.GetMagicToken() << " (dem v" << m_Header.GetDemoProtocol() << ", net v" << m_Header.GetNetworkProtocol() << ")\n"
		<< "\tGame:    " << m_Header.GetGameDirectory() << '\n'
		<< "\tMap:     " << m_Header.GetMapName() << '\n'
		<< "\tServer:  " << m_Header.GetServerName() << '\n'
		<< "\tClient:  " << m_Header.GetClientName() << '\n'
		<< "\tLength (seconds): " << m_Header.GetPlaybackTime() << '\n'
		<< "\tLength (ticks):   " << m_Header.GetPlaybackTicks() << '\n'
		<< "\tLength (frames):  " << m_Header.GetPlaybackFrames() << '\n'
		<< "\tSignon length:    " << m_Header.GetSignonLength() << '\n'
		<< cc::endl;

	ReadCommands(reader, m_Commands);

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << STR_FILEBITS(reader) << cc::fg::green << cc::bold << "dem_stop" << cc::endl;
}

void DemoFile::WriteElementInternal(BitIOWriter& writer) const
{
	m_Header.WriteElement(writer);

	for (const auto& cmd : m_Commands)
	{
		writer.Write(cmd->GetType());
		cmd->WriteElement(writer);

		assert(writer.GetPosition().IsByteAligned());
	}

	writer.Write(DemoCommandType::dem_stop);  // End of file
}

void DemoFile::ApplyWorldState(WorldState& world) const
{
	for (const auto& cmd : m_Commands)
		cmd->ApplyWorldState(world);
}