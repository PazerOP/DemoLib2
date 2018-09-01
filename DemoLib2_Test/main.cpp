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

#if false
	const auto& currentTable = world.GetStringTable(KnownStringTable::StaticBaselines);
	auto msg = std::make_shared<NetUpdateStringTableMessage>(*currentTable, KnownStringTable::StaticBaselines);

	auto currentIndex = currentTable->FindLowestUnused();

	for (const auto& item : std::filesystem::recursive_directory_iterator("staticbaselines/"))
	{
		auto indexString = item.path().filename().u8string();
		const auto index = atoi(indexString.c_str());

		bool found = false;
		for (auto& string : *currentTable)
		{
			if (string.GetString() == indexString)
			{
				found = true;
				break;
			}
		}

		if (found)
			continue;

		BitIOWriter writer(true);
		writer.Write(false); // "empty" static baseline
		writer.PadToByte();
		writer.SeekBits(0, Seek::Start);

		msg->GetUpdate().GetEntries().emplace_back(currentIndex, StringTableEntry(std::move(indexString), std::move(writer)));

		cc::out << cc::fg::green << "Added missing static baseline " << index << " at position " << currentIndex << cc::endl;

		currentIndex++;
	}

	for (const auto& entry : *currentTable)
	{
		auto& updates = msg->GetUpdate().GetEntries();
		updates.emplace_back(std::distance(currentTable->begin(), &entry), StringTableEntry());
	}

	return msg;
#endif

#if 0
	uint_fast16_t lastIndex = update.GetEntryCount() + 100;// update.GetMaxEntries() - 5;
	for (const auto& item : std::filesystem::recursive_directory_iterator("staticbaselines/"))
	{
		auto indexString = item.path().filename().u8string();
		const auto index = atoi(indexString.c_str());

		assert(index != 0);

		bool found = false;
		for (auto& entry : update.GetEntries())
		{
			auto parsed = atoi(entry.second.GetString().c_str());
			assert(parsed != 0);
			if (parsed == index)
			{
				found = true;
				break;
			}
		}

		if (found)
			continue;

		/*std::ifstream infile(item.path());
		infile.seekg(0, std::ios::end);
		const auto length = infile.tellg();
		infile.seekg(0);

		std::byte buffer[4096];
		assert(length < sizeof(buffer));
		infile.read((char*)buffer, length);

		BitIOWriter writer(true);
		writer.WriteChars((const char*)buffer, length);
		writer.Seek(BitPosition::Zero(), Seek::Set);

		update.GetEntries().emplace_back(lastIndex, StringTableEntry(std::move(indexString), std::move(writer)));

		cc::out << cc::fg::green << "Added missing static baseline " << index << " at position " << lastIndex << cc::endl;

		lastIndex++;*/

		BitIOWriter writer(true);
		writer.Write(false); // "empty" static baseline
		writer.PadToByte();
		writer.SeekBits(0, Seek::Start);

		update.GetEntries().emplace_back(lastIndex, StringTableEntry(std::move(indexString), std::move(writer)));

		cc::out << cc::fg::green << "Added missing static baseline " << index << " at position " << lastIndex << cc::endl;

		lastIndex++;
	}
#endif
}

static int ParseDemo(const std::string& inFile, const std::string& outFile)
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

#if 0
	DemoFile demo;
	demo.ReadElement(reader);

	{
		auto world = WorldState::Create();
		std::shared_ptr<UserReporter> userReporter;
		if (GetCmdArgs().m_PrintUsers)
			userReporter = UserReporter::Create(world);

		demo.ApplyWorldState(*world);
	}

	if (!outFile.empty())
	{
		cc::out << C_CYN C_BOLD "Encoding output demo..." C_RESET << std::endl;
		BitIOWriter writer(true);
		demo.WriteElement(writer);
		cc::out << C_CYN "Writing output demo (" << outFile << ')' << C_RESET << std::endl;
		{
			std::ofstream outDemo(outFile);
			writer.Seek(BitPosition::Zero(), Seek::Start);

			outDemo.write(reinterpret_cast<const char*>(writer.GetPtr()), writer.Length().Bytes());
		}
		cc::out << C_GRN C_BOLD "Completed demo save. Wrote " << writer.Length().TotalBytes() << " bytes to output demo." C_RESET << std::endl;
	}
	else
	{
		cc::out << "No outfile specified (-o), not saving any output." << std::endl;
	}
#else

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

#if false
			if (auto packet = dynamic_cast<DemoPacketCommand*>(cmd.get()))
			{
				for (int i = 0; i < packet->GetMessages().size(); i++)
				{
					auto& msg = packet->GetMessages()[i];
					if (!msg)
						continue;

					if (msg->GetType() == NetMessageType::SVC_CREATESTRINGTABLE)
					{
						auto createMsg = static_cast<NetCreateStringTableMessage*>(msg.get());
						if (createMsg->GetTableName() != "instancebaseline")
							continue;

						std::vector<std::pair<uint_fast16_t, StringTableEntry>>& entries = createMsg->GetTableUpdate().GetEntries();
						entries.erase(entries.begin(), entries.begin() + 25);
						continue;

						if (auto newMsg = AddMissingStaticBaselines(*world))
						{
							packet->GetMessages().push_back(newMsg);
							newMsg->ApplyWorldState(*world);
							assert(std::sin(0.5) > 0);
							i++;
							continue;
						}
					}
					else if (msg->GetType() == NetMessageType::SVC_UPDATESTRINGTABLE)
					{
						auto updateMsg = static_cast<NetUpdateStringTableMessage*>(msg.get());
						if (updateMsg->GetTableID() != (int)KnownStringTable::StaticBaselines)
							continue;

						packet->GetMessages().erase(packet->GetMessages().begin() + i);
						continue;

						if (auto newMsg = AddMissingStaticBaselines(*world))
						{
							packet->GetMessages().push_back(newMsg);
							newMsg->ApplyWorldState(*world);
							assert(std::sin(0.5) > 0);
							i++;
							continue;
						}
					}
				}
			}
			else
			{
				assert(cmd->GetType() != DemoCommandType::dem_signon);
			}
#endif

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
#endif

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

	int retVal;

#if RELEASE
	try
#endif
	{
		retVal = ParseDemo(GetCmdArgs().m_InFile, GetCmdArgs().m_OutFile);
	}
#if RELEASE
	catch (const std::exception& except)
	{
		cc::err << C_RED << C_BOLD << except.what() << C_RESET << std::endl;
		throw;
	}
#endif

	cc::out << cc::fg::cyan << "Finshed test run." << cc::endl;

	return retVal;
}