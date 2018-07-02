#include "PlayerPositionListener.hpp"

#include "data/TFTeam.hpp"
#include "misc/StrBuf.hpp"
#include "net/entities/Entity.hpp"
#include "net/worldstate/WorldState.hpp"

#include <fstream>
#include <iomanip>
#include <mutex>
#include <string_view>

using namespace std::chrono_literals;
using namespace std::string_view_literals;

std::atomic<uint_fast8_t> PlayerPositionListener::s_WaitingMerges;
std::atomic<uint_fast8_t> PlayerPositionListener::s_WaitingSaves;

std::once_flag PlayerPositionListener::s_IOCTStarted;
bool PlayerPositionListener::s_IOCTShuttingDown = false;
std::thread PlayerPositionListener::s_IOCoordinatorThread;
std::map<std::string, std::unique_ptr<PlayerPositionListener::PositionMapQueueEntry>> PlayerPositionListener::s_PositionMapQueue;
std::shared_mutex PlayerPositionListener::s_PositionMapQueueMutex;

PlayerPositionListener::PlayerPositionListener(const std::shared_ptr<WorldState>& world) : m_World(world)
{
	world->m_Events.AddEventListener(this);

	std::call_once(s_IOCTStarted, []
	{
		s_IOCoordinatorThread = std::thread(&PlayerPositionListener::IOCTFunc);
	});
}

PlayerPositionListener::~PlayerPositionListener()
{
	m_World->m_Events.RemoveEventListener(this);

	if (m_Positions.empty())
		return;

	char fileNameBuf[128];
	make_strbuf(fileNameBuf) << "maps/" << m_World->m_ServerInfo->m_MapName << ".cehm";
	m_World.reset();    // Literally the only thing we need m_World for is the mapname

	PositionMapQueueEntry* entry;

	// Find or create entry
	std::shared_lock<decltype(s_PositionMapQueueMutex)> readLock(s_PositionMapQueueMutex);
	std::unique_lock<decltype(s_PositionMapQueueMutex)> writeLock(s_PositionMapQueueMutex, std::defer_lock);
	{
		if (auto found = s_PositionMapQueue.find(fileNameBuf); found != s_PositionMapQueue.end())
			entry = found->second.get();
		else
		{
			// Upgrade lock
			readLock.unlock();
			writeLock.lock();

			// Try again, see if we got added during the lock upgrade above
			if (found = s_PositionMapQueue.find(fileNameBuf); found != s_PositionMapQueue.end())
				entry = found->second.get();
			else
			{
				// Looks like we really do need to add a new entry
				auto newEntry = std::make_unique<PositionMapQueueEntry>();
				entry = newEntry.get();
				s_PositionMapQueue[fileNameBuf] = std::move(newEntry);
			}
		}
	}

	// Lock entry and store our data
	{
		std::lock_guard<decltype(entry->m_InsertionMutex)> lock(entry->m_InsertionMutex);

		entry->m_WaitingMaps.emplace_back(std::move(m_Positions));
		entry->m_LastMapAddTime = std::chrono::steady_clock::now();
		s_WaitingMerges++;
	}
}

void PlayerPositionListener::ShutdownIOCThread()
{
	s_IOCTShuttingDown = true;
	s_IOCoordinatorThread.join();
}

void PlayerPositionListener::EntityCreated(const std::shared_ptr<Entity>& ent)
{
	if (!ent->Is("DT_TFPlayer"))
		return;

	Player newPlayer;
	newPlayer.m_Entity = ent;
	newPlayer.m_Team = &(*ent)["DT_BaseEntity.m_iTeamNum"sv].Get<TFTeam>();
	newPlayer.m_IsDead = &(*ent)["DT_PlayerState.deadflag"sv].Get<bool>();
	m_Players.push_back(newPlayer);
}

static Vector GetPlayerPosition(const Entity& player, uint_fast32_t& lastUpdate)
{
#define LOCALNAME "DT_TFLocalPlayerExclusive.m_vecOrigin"
#define NONLOCALNAME "DT_TFNonLocalPlayerExclusive.m_vecOrigin"

	const auto& localxy = player[LOCALNAME];
	const auto& nonLocalxy = player[NONLOCALNAME];

	Vector retVal = vec3_nan;

	if (localxy.GetLastChangedTick() > 0 && localxy.GetLastChangedTick() > nonLocalxy.GetLastChangedTick())
	{
		const auto& localz = player[LOCALNAME "[2]"];

		retVal.SetXY(localxy.Get<VectorXY>());
		retVal.z = localz.Get<float>();

		lastUpdate = std::max(localxy.GetLastChangedTick(), localz.GetLastChangedTick());
	}
	else if (nonLocalxy.GetLastChangedTick() > 0)
	{
		const auto& nonLocalz = player[NONLOCALNAME "[2]"];

		retVal.SetXY(nonLocalxy.Get<VectorXY>());
		retVal.z = nonLocalz.Get<float>();

		lastUpdate = std::max(nonLocalxy.GetLastChangedTick(), nonLocalz.GetLastChangedTick());
	}

	return retVal;

#undef LOCALNAME
#undef NONLOCALNAME
}

void PlayerPositionListener::TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta)
{
	for (auto iter = m_Players.begin(); iter != m_Players.end(); )
	{
		if (auto locked = iter->m_Entity.lock())
		{
			if (!locked->IsInPVS())
				goto Continue;

			if (*iter->m_Team != TFTeam::Red && *iter->m_Team != TFTeam::Blue)
				goto Continue;

			if (*iter->m_IsDead)
				goto Continue;

			{
				uint_fast32_t lastUpdate;
				auto pos = GetPlayerPosition(*locked, lastUpdate);
				if (lastUpdate >= (world->m_Tick - delta))
					m_Positions[int16_t(pos.z / SCALE)][int16_t(pos.y / SCALE)][int16_t(pos.x / SCALE)]++;
			}

			Continue:
			++iter;
		}
		else
		{
			iter = m_Players.erase(iter);
		}
	}
}

void PlayerPositionListener::IOCTFunc()
{
	std::vector<std::pair<std::string, PositionMapQueueEntry*>> queueEntries;
	while (!s_IOCTShuttingDown || s_PositionMapQueue.size())
	{
		if (!s_IOCTShuttingDown)
			std::this_thread::sleep_for(1s);

		// Make a copy of the queue entries into a temporary vector so we don't have to keep it locked the whole time
		queueEntries.clear();
		{
			bool needsWriteLock = false;
			{
				std::unique_lock<decltype(s_PositionMapQueueMutex)> readLock(s_PositionMapQueueMutex);
				for (auto& queueObj : s_PositionMapQueue)
				{
					if (queueObj.second->ShouldPurge())
					{
						needsWriteLock = true;
						break;
					}
					else
						queueEntries.push_back(std::make_pair(queueObj.first, queueObj.second.get()));
				}
			}

			if (needsWriteLock)
			{
				queueEntries.clear();

				// With a write (exclusive) lock, purge any entries that are too old.
				std::shared_lock<decltype(s_PositionMapQueueMutex)> writeLock(s_PositionMapQueueMutex, std::defer_lock);
				for (auto iter = s_PositionMapQueue.begin(); iter != s_PositionMapQueue.end(); )
				{
					if (iter->second->ShouldPurge())
					{
						cc::out << cc::fg::cyan << "Purging position map queue entry for " << iter->first << cc::endl;
						iter = s_PositionMapQueue.erase(iter);
					}
					else
					{
						queueEntries.push_back(std::make_pair(iter->first, iter->second.get()));
						++iter;
					}
				}
			}
		}

		for (auto queueObj : queueEntries)
		{
			bool isSyncing = false;
			// Merge any into the main PositionMap
			{
				std::lock_guard<decltype(queueObj.second->m_InsertionMutex)> lock(queueObj.second->m_InsertionMutex);

				if (queueObj.second->m_WaitingMaps.empty())
				{
					if (queueObj.second->ShouldSave())
					{
						// We're going to sync to disk below
						isSyncing = true;
						queueObj.second->m_SyncedToDisk = true;
					}
				}
				else
				{
					const auto waitingMapCount = queueObj.second->m_WaitingMaps.size();

					auto& mergedMaps = queueObj.second->m_MergedMaps;
					if (mergedMaps.empty() && !queueObj.second->m_WaitingMaps.empty())
					{
						mergedMaps = std::move(queueObj.second->m_WaitingMaps.back());
						queueObj.second->m_WaitingMaps.pop_back();
					}

					for (const auto& childMap : queueObj.second->m_WaitingMaps)
					{
						for (const auto& zPair : childMap)
						{
							for (const auto& yPair : zPair.second)
							{
								for (const auto& xPair : yPair.second)
									mergedMaps[zPair.first][yPair.first][xPair.first] += xPair.second;
							}
						}
					}

					if (queueObj.second->m_SyncedToDisk)
					{
						// We're basically queuing up a save
						s_WaitingSaves++;
						queueObj.second->m_SyncedToDisk = false;
					}

					// Release any memory from waiting maps
					queueObj.second->m_WaitingMaps.clear();

					s_WaitingMerges -= (uint_fast8_t)waitingMapCount;
				}
			}

			// Should we sync to disk?
			if (isSyncing)
			{
				// Read existing file from disk
				if (!queueObj.second->m_MergedFromDisk)
				{
					ReadPositionMapFromDisk(queueObj.first.c_str(), queueObj.second->m_MergedMaps);
					queueObj.second->m_MergedFromDisk = true;
				}

				// Write new file to disk
				WritePositionMapToDisk(queueObj.first.c_str(), queueObj.second->m_MergedMaps);

				s_WaitingSaves--;
			}
		}
	}
}

void PlayerPositionListener::ReadPositionMapFromDisk(const char* fileName, PositionMap& pMap)
{
	cc::out << cc::fg::white << "Reading previous results from " << fileName << "...";
	if (std::ifstream input(fileName, std::ios::binary); input.good())
	{
		HeatmapFileHeader header;
		input >> header;

		if (header.m_MagicBytes != HeatmapFileHeader::MAGIC_BYTES)
		{
			cc::out << cc::fg::red << " invalid heatmap file header." << cc::endl;
			return;
		}

		if (header.m_Version != HeatmapFileHeader::VERSION)
		{
			cc::out << cc::fg::red << " unknown heatmap file version." << cc::endl;
			return;
		}

		HeatmapFileEntry entry;
		for (uint32_t i = 0; i < header.m_EntryCount; i++)
		{
			input >> entry;

			pMap[entry.z][entry.y][entry.x] += entry.count;
		}

		cc::out << cc::fg::green << " done!" << cc::endl;
	}
	else
	{
		cc::out << " file not found." << cc::endl;
	}
}

void PlayerPositionListener::WritePositionMapToDisk(const char* fileName, const PositionMap& pMap)
{
	cc::out << cc::fg::white << "Writing results to " << fileName << "...";
	if (std::ofstream output(fileName, std::ios::binary); output.good())
	{
		const auto startPos = output.tellp();
		output.seekp(HeatmapFileHeader::SIZE);

		uint32_t entryCount = 0;

		HeatmapFileEntry entry;
		for (const auto& zPair : pMap)
		{
			entry.z = zPair.first;

			for (const auto& yPair : zPair.second)
			{
				entry.y = yPair.first;

				for (const auto& xPair : yPair.second)
				{
					entry.x = xPair.first;
					entry.count = xPair.second;

					output << entry;
					entryCount++;
				}
			}
		}

		output.seekp(startPos);
		output << HeatmapFileHeader(entryCount);

		cc::out << cc::fg::green << " done!" << cc::endl;
	}
	else
	{
		cc::out << cc::fg::red << " failed! Unable to open output file." << cc::endl;
	}
}

bool PlayerPositionListener::PositionMapQueueEntry::ShouldSave() const
{
	if (m_SyncedToDisk)
		return false;

	if (s_IOCTShuttingDown)
		return true;

	return (std::chrono::steady_clock::now() - m_LastMapAddTime) > 10s;
}

bool PlayerPositionListener::PositionMapQueueEntry::ShouldPurge() const
{
	if (!m_SyncedToDisk)
		return false;
	if (!m_WaitingMaps.empty())
		return false;

	if (s_IOCTShuttingDown)
		return true;

	return (std::chrono::steady_clock::now() - m_LastMapAddTime) > 30s;
}
