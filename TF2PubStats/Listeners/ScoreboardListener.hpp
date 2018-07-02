#pragma once
#include "data/RoundState.hpp"
#include "data/TFClassType.hpp"
#include "data/TFTeam.hpp"
#include "net/worldstate/WorldState.hpp"

#include <chrono>
#include <map>
#include <string>

class IPropertySet;

class ScoreboardListener final : IWorldEventListener
{
public:
	ScoreboardListener(const std::shared_ptr<WorldState>& world);
	~ScoreboardListener();

	void PrintConsoleOutput() const;

protected:
	void EntityCreated(const std::shared_ptr<Entity>& ent) override;
	void GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event) override;
	void TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta) override;

private:
	std::weak_ptr<WorldState> m_World;
	WorldState& GetWS() const { return *m_World.lock(); }

	bool FindPlayerNameByUserID(uint_fast16_t userID, std::string& name) const;

	void OnGameRulesDeleted(const std::shared_ptr<Entity>&);

	void UpdatePlayerTeams(WorldState& world);
	void UpdatePlayerDPMs(uint_fast32_t tickDelta);

	std::string BuildScoreboard() const;

	void OnPlayerHurt(const GameEvent& event);
	void OnPlayerHealed(const GameEvent& event);
	void OnPlayerDeath(const GameEvent& event);
	void OnPlayerSpawn(const GameEvent& event);

	struct PlayerEventData
	{
		uint_fast16_t GetDPM() const;

		uint_fast16_t m_Kills = 0;
		uint_fast16_t m_Deaths = 0;
		uint_fast16_t m_Killstreak = 0;
		uint_fast32_t m_Damage = 0;
		uint_fast32_t m_DamageTaken = 0;
		uint_fast32_t m_Healing = 0;
		uint_fast32_t m_RoundTicks = 0;

		TFTeam m_Team = TFTeam::Unassigned;
		TFClassType m_Class = TFClassType::Unknown;
	};

	const RoundState* m_RoundState = nullptr;
	const int* m_bInSetup = nullptr;

	std::chrono::time_point<std::chrono::steady_clock> m_LastConsoleUpdate;
	std::map<uint_fast16_t, PlayerEventData> m_Stats;
};