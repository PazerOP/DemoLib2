#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "demos/DemoFile.hpp"
#include "demos/commands/IDemoCommand.hpp"
#include "demos/commands/DemoPacketCommand.hpp"
#include "demos/commands/DemoStringTablesCommand.hpp"
#include "ExtendedCmdArgs.hpp"
#include "reporters/GameEventReporter.hpp"
#include "reporters/UserReporter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/KnownStringTable.hpp"
#include "net/netmessages/INetMessage.hpp"
#include "net/netmessages/NetCreateStringTableMessage.hpp"
#include "net/netmessages/NetUpdateStringTableMessage.hpp"
#include "net/worldstate/WorldState.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>

static void DumpStaticBaselines(const WorldState& world)
{
	const auto& staticBaselinesTable = world.GetStringTable(KnownStringTable::StaticBaselines);

	for (auto& entry : *staticBaselinesTable)
	{
		auto reader = entry.GetUserDataReader();
		assert(reader.GetLocalPosition().IsZero());

		const auto bytes = reader.Length().Bytes();
		assert(reader.Length().Bits() == 0);

		char filename[128];
		sprintf_s(filename, "staticbaselines/%s", entry.GetString().c_str());
		std::ofstream outFile(filename);

		std::byte buffer[4096];
		assert(bytes <= sizeof(buffer));
		reader.ReadArray(buffer, bytes);

		outFile.write((const char*)buffer, bytes);
	}
}

static void AddMissingStaticBaselines(const WorldState& world, DemoStringTablesCommand& cmd)
{
	auto& baselineTable = cmd.GetTables().at("instancebaseline");

	// Add about 100 entries of padding
	for (int i = 0; i < 100; i++)
		baselineTable.emplace_back();

	for (uint_fast16_t serverClassIndex = 0; serverClassIndex < world.m_ServerClasses.size(); serverClassIndex++)
	{
		if (auto found = std::find_if(baselineTable.begin(), baselineTable.end(),
			[serverClassIndex](const StringTableEntry& a) { return atoi(a.GetString().c_str()) == serverClassIndex; });
			found != baselineTable.end())
		{
			continue;
		}

		BitIOWriter writer(true);
		writer.Write(false); // "empty" static baseline
		writer.PadToByte();
		writer.SeekBits(0, Seek::Start);

		char buf[64];
		sprintf_s(buf, "%i", serverClassIndex);
		baselineTable.emplace_back(buf, std::move(writer));

		cc::out << cc::fg::green << "Added missing static baseline " << serverClassIndex << " at position " << baselineTable.size() - 1 << cc::endl;
	}
}

static int ParseDemo(const std::string_view& inFile, const std::string_view& outFile)
{
	cc::out << "test: sizeof(WorldState) = " << sizeof(WorldState) << " (" << sizeof(WorldState) / 1024 << " KB)" << std::endl;

	size_t dataLength;
	std::shared_ptr<std::byte[]> data;

	// Read whole file into ram
	{
		cc::out << cc::fg::cyan << cc::bold << "Loading " << inFile << "..." << cc::reset << std::endl;
		std::ifstream testDemo(inFile, std::ios::binary);
		if (!testDemo.good())
		{
			cc::out << cc::fg::red << cc::bold << "Unable to open input file " << inFile << " for read." << cc::reset << std::endl;
			return 1;
		}

		// Determine length so we can allocate memory
		testDemo.seekg(0, std::ifstream::end);
		dataLength = testDemo.tellg();
		cc::out << cc::fg::cyan << "Loading " << dataLength << " bytes into RAM..." << cc::reset << std::endl;

		// Allocate memory and load into ram
		data.reset(new std::byte[dataLength]);
		testDemo.seekg(0, std::ifstream::beg);
		testDemo.read((char*)data.get(), dataLength);

		auto pos = testDemo.tellg();
		assert((size_t)pos == dataLength);
	}

	cc::out << cc::fg::cyan << cc::bold << "Decoding demo..." << cc::reset << std::endl;
	BitIOReader reader(data, BitPosition::FromBytes(dataLength));

	{
		DemoHeader header;
		header.ReadElement(reader);

		auto world = WorldState::Create();
		std::unique_ptr<UserReporter> userReporter;
		if (GetCmdArgs().m_PrintUsers)
			userReporter = std::make_unique<UserReporter>(world);

		//auto gameEventReporter = std::make_unique<GameEventReporter>(world);

		BitIOWriter writer(true);
		header.SetSignonLength(0);
		header.WriteElement(writer);

		while (auto cmd = DemoFile::ReadCommand(reader))
		{
			if (cmd->GetType() == DemoCommandType::dem_stringtables)
				AddMissingStaticBaselines(*world, *static_cast<DemoStringTablesCommand*>(cmd.get()));

			cmd->ApplyWorldState(*world);
			writer.Write(cmd->GetType());
			cmd->WriteElement(writer);
		}

		writer.Write(DemoCommandType::dem_stop);

		writer.PadToByte();
		cc::out << "Writing output demo (" << outFile << ')' << cc::endl;
		{
			std::ofstream outDemo(outFile, std::ios::binary);
			writer.Seek(BitPosition::Zero(), Seek::Start);

			outDemo.write(reinterpret_cast<const char*>(writer.GetPtr()), writer.Length().Bytes());
		}

		//DumpStaticBaselines(*world);
	}

	return 0; // No errors if we made it here
}

int main(int argc, char** argv)
{
	if (!ParseCmdArgs(argc, argv))
		return 1;

	if (GetCmdArgs().m_InFile.empty())
	{
		cc::out << cc::fg::red << cc::bold << "No input file specified (use -i <file>)" << cc::endl;
		return 1;
	}

#if RELEASE
	try
#endif
	{
		if (const std::filesystem::path path(GetCmdArgs().m_InFile); path.extension() == ".txt")
		{
			cc::out << "Processing list file " << path << "." << cc::endl;

			std::ifstream listfile(path);
			const auto basePath = std::filesystem::path(path).remove_filename();

			while (!listfile.eof())
			{
				char buf[512];
				listfile.getline(buf, std::size(buf));

				const auto inputPath = basePath / buf;

				const auto outputPath = std::filesystem::path(inputPath)
					.replace_filename(inputPath.filename().replace_extension().u8string() + "_REPAIRED.dem");

				if (auto result = ParseDemo(inputPath.u8string(), outputPath.u8string()))
					return result;
			}

			return 0;
		}
		else
		{
			return ParseDemo(GetCmdArgs().m_InFile, GetCmdArgs().m_OutFile);
		}
	}
#if RELEASE
	catch (const std::exception& except)
	{
		cc::err << C_RED << C_BOLD << except.what() << C_RESET << std::endl;
		throw;
	}
#endif

	cc::out << cc::fg::cyan << "Finshed test run." << cc::endl;
}