#include "ExtendedCmdArgs.hpp"
#include "demos/commands/DemoPacketCommand.hpp"
#include "demos/commands/IDemoCommand.hpp"
#include "demos/DemoFile.hpp"
#include "misc/Exceptions.hpp"
#include "misc/Semaphore.hpp"
#include "misc/StrBuf.hpp"
#include "net/entities/Entity.hpp"
#include "net/netmessages/INetMessage.hpp"
#include "net/netmessages/NetTickMessage.hpp"
#include "net/worldstate/WorldState.hpp"
#include "Listeners/PlayerPositionListener.hpp"
#include "Listeners/RandomCritListener.hpp"
#include "Listeners/ScoreboardListener.hpp"

#include <chrono>
#include <condition_variable>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <string_view>
#include <thread>

#include <Windows.h>

#undef max

//#define READ_FULL 1

using namespace std::chrono_literals;

class EventStressTest : IWorldEventListener
{
public:
	EventStressTest(const std::shared_ptr<WorldState>& world) : m_World(world)
	{
		world->m_Events.AddEventListener(this);
	}
	~EventStressTest()
	{
		if (auto locked = m_World.lock())
			locked->m_Events.RemoveEventListener(this);
	}

private:
	std::weak_ptr<WorldState> m_World;
};

class DemoAnalyzer
{
public:
	DemoAnalyzer();
	~DemoAnalyzer();

	static void MonitorConsoleLog(const std::string_view& logFile);
	void StaticAnalyze(const std::string_view& demFile);

	void ReadDemo(std::string&& demoFileName);

	const DemoHeader& GetDemoHeader() const { return m_DemoHeader; }

private:
	void UpdateConsoleThread(bool* shuttingDown, bool runOnce);

	bool StartReadingDemo(std::string&& demoFileName);
	bool HandleDemoUpdate(size_t until);

	bool CheckRecordingStart(const std::string_view& str);
	bool IsRecordingEnd(const std::string_view& str);

	std::string m_DemoFileName;
	std::unique_ptr<FILE, decltype(&fclose)> m_DemoFile;
	DemoHeader m_DemoHeader;

	std::shared_ptr<WorldState> m_WorldState;

	std::chrono::steady_clock::time_point m_LastDemoReadTime;
	size_t m_LastDemoReadPos = 0;

	std::unique_ptr<ScoreboardListener> m_ScoreboardListener;
	std::unique_ptr<RandomCritListener> m_RandomCritListener;
	std::unique_ptr<PlayerPositionListener> m_PlayerPositionListener;

	std::unique_ptr<EventStressTest> m_StressTestListeners[1];
};

DemoAnalyzer::DemoAnalyzer() : m_DemoFile(nullptr, &fclose)
{
}
DemoAnalyzer::~DemoAnalyzer()
{
	m_DemoFile.reset();
	m_WorldState.reset();
}

bool DemoAnalyzer::StartReadingDemo(std::string&& demoFileName)
{
	m_DemoFileName = std::move(demoFileName);
	m_DemoFile.reset(fopen(m_DemoFileName.c_str(), "rb"));

	m_LastDemoReadTime = std::chrono::steady_clock::now();

	cc::out << cc::fg::green << cc::bold << cc::ul << "Began processing " << m_DemoFileName << cc::endl;

	// Read in the demo header
	{
		size_t totalBytesRead = 0;
		std::shared_ptr<std::byte[]> array(new std::byte[DemoHeader::DEMO_HEADER_SIZE]);

		while (totalBytesRead < DemoHeader::DEMO_HEADER_SIZE)
		{
			const auto bytesRead = fread(array.get() + totalBytesRead, 1, DemoHeader::DEMO_HEADER_SIZE - totalBytesRead, m_DemoFile.get());
			totalBytesRead += bytesRead;

			fseek(m_DemoFile.get(), (long)totalBytesRead, SEEK_SET);
		}

		BitIOReader reader(array, BitPosition::FromBytes(DemoHeader::DEMO_HEADER_SIZE));
		m_DemoHeader.ReadElement(reader);

		m_LastDemoReadPos = DemoHeader::DEMO_HEADER_SIZE;

		//if (const auto& name = m_DemoHeader.GetMapName(); name[0] != 'p' || name[1] != 'l' || name[2] != '_' || name[3] != 'u')
		//	return false;
	}

	cc::out << cc::fg::dark::green << "Read demo header." << std::endl;

	// Init WorldState
	{
		m_WorldState = WorldState::Create();
		//m_ScoreboardListener = std::make_unique<ScoreboardListener>(m_WorldState);
		m_RandomCritListener = std::make_unique<RandomCritListener>(m_WorldState);
		//m_PlayerPositionListener = std::make_unique<PlayerPositionListener>(m_WorldState);

		for (auto& stress : m_StressTestListeners)
			stress = std::make_unique<EventStressTest>(m_WorldState);
	}

	cc::out << cc::fg::dark::green << "Initialized WorldState." << cc::endl;
	return true;
}

bool DemoAnalyzer::CheckRecordingStart(const std::string_view& str)
{
	static std::regex s_RecordingStartRegex("Recording to (.*)\\.\\.\\.", std::regex_constants::optimize);

	std::cmatch matches;
	if (!std::regex_search(str.data(), matches, s_RecordingStartRegex))
		return false;

	std::string demoFileName = GetCmdArgs().m_GameDir;
	demoFileName += std::string_view(matches[1].first, matches[1].second - matches[1].first);

	StartReadingDemo(std::move(demoFileName));

	return true;
}

static uint_fast32_t CheckDemoUpdate(const std::string_view& str)
{
	static std::regex s_WritingDemoMessageRegex("Writing demo message (\\d+) bytes at file pos (\\d+)", std::regex_constants::optimize);

	std::cmatch matches;
	if (!std::regex_match(str.data(), matches, s_WritingDemoMessageRegex))
		return 0;

	return strtoul(matches[2].first, nullptr, 10);
}

bool DemoAnalyzer::IsRecordingEnd(const std::string_view& str)
{
	static std::regex s_RecordingEndRegex("Completed demo, recording time (\\d+(?:\\.\\d+)?), game frames (\\d+)\\.", std::regex_constants::optimize);

	std::cmatch matches;
	if (!std::regex_search(str.data(), matches, s_RecordingEndRegex))
		return false;

	//s_DemoFileName.clear();
	m_DemoFile.reset();

	m_WorldState.reset();

	return true;
}

bool DemoAnalyzer::HandleDemoUpdate(size_t until)
{
	const bool isSignon = m_LastDemoReadPos == DemoHeader::DEMO_HEADER_SIZE;

	const auto delta = until - m_LastDemoReadPos;

	if (isSignon)
		cc::out << "Reading initial signon data (" << delta << " bytes)..." << cc::endl;

	int result;

	result = fseek(m_DemoFile.get(), (long)m_LastDemoReadPos, SEEK_SET);
	assert(!result);

	std::shared_ptr<std::byte[]> array(new std::byte[delta]);
	size_t totalBytesRead = 0;

	while (totalBytesRead < delta)
	{
		const auto bytesRead = fread(array.get() + totalBytesRead, 1, delta - totalBytesRead, m_DemoFile.get());
		totalBytesRead += bytesRead;

		if (!bytesRead)
		{
			std::this_thread::sleep_for(1s);
			continue;
		}

		result = fseek(m_DemoFile.get(), long(m_LastDemoReadPos + totalBytesRead), SEEK_SET);
		assert(!result);

		const auto endPosition = ftell(m_DemoFile.get());
		//assert(bytesRead);
		assert(!ferror(m_DemoFile.get()));
		assert(!feof(m_DemoFile.get()));
		assert(totalBytesRead <= delta);
	}

	//cc::out << "Read " << delta << " bytes  at " << s_LastDemoReadPos << std::endl;

	BitIOReader reader(array, BitPosition::FromBytes(delta));

#ifdef READ_FULL
	if (isSignon)
		cc::out << "Reading signon state..." << cc::endl;

	DemoFile::DemoCmdVec allCommands;
	DemoFile::ReadCommands(reader, allCommands);

	uint_fast32_t baseTick = 0;
	std::vector<std::pair<uint_fast32_t, uint_fast32_t>> tickPairs;

	for (const auto& baseCmd : allCommands)
	{
		if (baseCmd->GetType() != DemoCommandType::dem_packet)
			continue;

		auto cmd = static_cast<const DemoPacketCommand*>(baseCmd.get());
		for (const auto& baseMsg : cmd->GetMessages())
		{
			if (!baseMsg || baseMsg->GetType() != NetMessageType::NET_TICK)
				continue;

			auto msg = static_cast<const NetTickMessage*>(baseMsg.get());

			if (cmd->GetTick() == 0)
				baseTick = msg->GetTick();

			tickPairs.emplace_back(cmd->GetTick(), msg->GetTick() - baseTick);
		}
	}

	if (isSignon)
		cc::out << "Applying signon state..." << cc::endl;

	for (const auto& cmd : allCommands)
		cmd->ApplyWorldState(*m_WorldState);
#else
	if (isSignon)
		cc::out << "Reading/applying signon state..." << cc::endl;

	while (!reader.IsAtEnd())
	{
		auto cmd = DemoFile::ReadCommand(reader);
		if (!cmd)
			break;

		cmd->ApplyWorldState(*m_WorldState);
	}
#endif

	//for (const auto& cmd : allCommands)
	//	cmd->ApplyWorldState(*m_WorldState);

	if (isSignon)
		cc::out << cc::fg::green << "Finished processing signon state." << cc::endl;

	m_LastDemoReadPos = until;

	//cc::out << "s_LastDemoReadPos = " << s_LastDemoReadPos << std::endl;

	return true;
}

void DemoAnalyzer::UpdateConsoleThread(bool* shuttingDown, bool runOnce)
{
	if (!runOnce)
		cc::out << "Started console update thread" << std::endl;

	uint_fast32_t avgDeciSecondsBehind = 0;
	uint_fast32_t totalSamples = 0;

	uint_fast32_t prevDeciSecondsBehind = 0;

	while (!*shuttingDown || runOnce)
	{
		runOnce = false;
		std::this_thread::sleep_for(0.1s);

		if (m_ScoreboardListener)
		{
			if (!m_WorldState || !m_ScoreboardListener)
				continue;

			m_ScoreboardListener->PrintConsoleOutput();
		}

		if (m_DemoFile)
		{
			auto timeDist = std::chrono::duration_cast<std::chrono::duration<uint_fast32_t, std::deci>>(std::chrono::steady_clock::now() - m_LastDemoReadTime);

			if (timeDist.count() < prevDeciSecondsBehind)
			{
				avgDeciSecondsBehind += prevDeciSecondsBehind;
				totalSamples++;
			}

			prevDeciSecondsBehind = timeDist.count();

			if (totalSamples > 0)
			{
				char buf[64];
				sprintf_s(buf, "TF2PubStats - average %i.%i seconds behind game (current %i.%i)",
					(avgDeciSecondsBehind / totalSamples) / 10, (avgDeciSecondsBehind / totalSamples) % 10,
					timeDist.count() / 10, timeDist.count() % 10);
				SetConsoleTitleA(buf);
			}
		}
		else
		{
			SetConsoleTitleA("TF2PubStats - Waiting");
			avgDeciSecondsBehind = 0;
			totalSamples = 0;
		}
	}
}

void DemoAnalyzer::MonitorConsoleLog(const std::string_view& logfile)
{
	// Start another thread to handle console updates
	std::unique_ptr<FILE, decltype(&fclose)> conlog(fopen(logfile.data(), "r"), &fclose);
	fseek(conlog.get(), 0, SEEK_END);

	std::string buf;
	bool shuttingDown = false;
	while (true)
	{
		std::this_thread::sleep_for(500ms);

		DemoAnalyzer analyzer;
		shuttingDown = false;
		std::thread consoleThread(&DemoAnalyzer::UpdateConsoleThread, &analyzer, &shuttingDown, false);

		char readChar;
		while ((readChar = fgetc(conlog.get())) != EOF)
		{
			if (readChar == '\n')
			{
				if (analyzer.CheckRecordingStart(buf))
				{
				}
				else if (analyzer.m_DemoFile)
				{
					if (auto until = CheckDemoUpdate(buf))
					{
						//cc::out << "Demo file position: " << s_DemoEndPos << std::endl;
						analyzer.HandleDemoUpdate(until);
						analyzer.m_LastDemoReadTime = std::chrono::steady_clock::now();
						//cc::out << buf << std::endl;
					}
					else if (analyzer.IsRecordingEnd(buf))
					{
						cc::out << "Finished processing " << analyzer.m_DemoFileName << std::endl;
						break;
					}
				}

				buf.clear();
			}
			else
			{
				buf += readChar;
			}
		}

		shuttingDown = true;
	}
}

void DemoAnalyzer::ReadDemo(std::string&& demoFileName)
{
	if (!StartReadingDemo(std::move(demoFileName)))
		return;

	fseek(m_DemoFile.get(), 0, SEEK_END);
	const auto length = ftell(m_DemoFile.get());
	HandleDemoUpdate((size_t)length);	// Pretend it's one big demo update
}

void DemoAnalyzer::StaticAnalyze(const std::string_view& demFile)
{
	//cc::out << "Performing static analysis of " << demFile << std::endl;

	try
	{
		ReadDemo(std::string(demFile));

		bool shuttingDown = true;
		UpdateConsoleThread(&shuttingDown, true);
	}
	catch (const std::out_of_range& ex)
	{
		cc::out << cc::reset << cc::fg::red << "Failed to parse demo: " << ex.what() << cc::endl;
	}
}

static std::atomic<uint32_t> s_ProcessedDuration;
static void ConsoleTitleUpdateThread(const bool* shuttingDown)
{
	while (!*shuttingDown)
	{
		const uint32_t processedDuration = s_ProcessedDuration;

		char buf[128];
		make_strbuf(buf)
			<< "Waiting merges: " << +PlayerPositionListener::GetWaitingMerges()
			<< ", Waiting saves: " << +PlayerPositionListener::GetWaitingSaves()
			<< ", Processed " << processedDuration / 86400 << ':'
			<< std::setw(2) << std::setfill('0') << (processedDuration / 3600) % 24 << ':'
			<< std::setw(2) << std::setfill('0') << (processedDuration / 60) % 60 << ':'
			<< std::setw(2) << std::setfill('0') << processedDuration % 60;

		SetConsoleTitleA(buf);

		std::this_thread::sleep_for(1s);
	}
}

int main(int argc, char** argv)
{
	if (!ParseCmdArgs(argc, argv))
		return 1;

	if (!GetCmdArgs().m_StaticAnalyzeFile.empty())
	{
		DemoAnalyzer analyzer;
		analyzer.StaticAnalyze(GetCmdArgs().m_StaticAnalyzeFile);
	}
	else if (!GetCmdArgs().m_StaticAnalyzeDir.empty())
	{
		bool titleUpdateThreadShuttingDown = false;
		std::thread titleUpdateThread(&ConsoleTitleUpdateThread, &titleUpdateThreadShuttingDown);

		WIN32_FIND_DATAA ffd;
		const auto& dir = GetCmdArgs().m_StaticAnalyzeDir + "\\*.dem";
		HANDLE hFind = FindFirstFileA(dir.c_str(), &ffd);

		static const auto THREAD_COUNT = 1;// std::max<uint32_t>(std::thread::hardware_concurrency() /* - 1 */, 1);
		Semaphore semaphore(THREAD_COUNT);
		std::vector<std::future<void>> futures;

		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			semaphore.Wait();

			auto future = std::async(std::launch::async | std::launch::deferred,
				[&semaphore](std::string fileName)
			{
				SemaphoreAutoNotify autoNotify(semaphore);

				DemoAnalyzer analyzer;
				analyzer.StaticAnalyze(fileName);

				s_ProcessedDuration += std::lround(analyzer.GetDemoHeader().GetPlaybackTime());

			}, GetCmdArgs().m_StaticAnalyzeDir + '\\' + ffd.cFileName);

			// Find somewhere to put our future so we don't block on future destructor
			for (auto& potential : futures)
			{
				if (!potential.valid() || potential.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
				{
					// Put our future here
					potential = std::move(future);
					break;
				}
			}

			if (future.valid())
			{
				if (futures.size() >= THREAD_COUNT)
					cc::out << cc::bg::yellow << cc::fg::black << "Growing futures vector... (current size: " << futures.size() << ')' << cc::endl;

				futures.emplace_back(std::move(future));
			}

		} while (FindNextFileA(hFind, &ffd));

		FindClose(hFind);

		titleUpdateThreadShuttingDown = true;
		titleUpdateThread.join();
	}
	else
	{
		DemoAnalyzer analyzer;
		analyzer.MonitorConsoleLog(GetCmdArgs().m_ConLogFile);
	}

	PlayerPositionListener::ShutdownIOCThread();
	return 0;
}

