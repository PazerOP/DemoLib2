#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "demos/DemoFile.hpp"
#include "demos/commands/IDemoCommand.hpp"
#include "ExtendedCmdArgs.hpp"
#include "reporters/GameEventReporter.hpp"
#include "reporters/UserReporter.hpp"
#include "misc/Exceptions.hpp"

#include "net/worldstate/WorldState.hpp"

#include <fstream>
#include <memory>
#include <iostream>

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

		auto gameEventReporter = std::make_unique<GameEventReporter>(world);

		std::shared_ptr<IDemoCommand> cmd;
		while (cmd = DemoFile::ReadCommand(reader))
		{
			cmd->ApplyWorldState(*world);
		}
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