#pragma once

#include "data/HeatmapFile.hpp"
#include "net/worldstate/IWorldEventListener.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

enum class TFTeam;

class PlayerPositionListener final : IWorldEventListener
{
public:
	PlayerPositionListener(const std::shared_ptr<WorldState>& world);
	~PlayerPositionListener();

	static uint_fast8_t GetWaitingMerges() { return s_WaitingMerges; }
	static uint_fast8_t GetWaitingSaves() { return s_WaitingSaves; }

	static void ShutdownIOCThread();

protected:
	void EntityCreated(const std::shared_ptr<Entity>& ent) override;
	void TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta) override;

private:
	static constexpr auto SCALE = 16;

	// z, y, x
	using PositionMap = std::unordered_map<int16_t, std::unordered_map<int16_t, std::unordered_map<int16_t, uint_fast32_t>>>;
	PositionMap m_Positions;

	static std::atomic<uint_fast8_t> s_WaitingMerges;
	static std::atomic<uint_fast8_t> s_WaitingSaves;

	static std::once_flag s_IOCTStarted;
	static std::thread s_IOCoordinatorThread;
	static bool s_IOCTShuttingDown;
	static void IOCTFunc();
	static void ReadPositionMapFromDisk(const char* fileName, PositionMap& pMap);
	static void WritePositionMapToDisk(const char* fileName, const PositionMap& pMap);

	struct PositionMapQueueEntry
	{
		bool m_MergedFromDisk = false;
		bool m_SyncedToDisk = true;
		PositionMap m_MergedMaps;

		std::mutex m_InsertionMutex;
		std::chrono::steady_clock::time_point m_LastMapAddTime;
		std::vector<PositionMap> m_WaitingMaps;

		bool ShouldSave() const;
		bool ShouldPurge() const;
	};
	static std::map<std::string, std::unique_ptr<PositionMapQueueEntry>> s_PositionMapQueue;
	static std::shared_mutex s_PositionMapQueueMutex;

	struct Player
	{
		std::weak_ptr<Entity> m_Entity;
		const TFTeam* m_Team;
		const bool* m_IsDead;
	};

	std::shared_ptr<WorldState> m_World;
	std::vector<Player> m_Players;
};