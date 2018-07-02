#pragma once
#include "data/DamageBits.hpp"
#include "data/RoundState.hpp"
#include "data/TFClassType.hpp"
#include "data/TFCond.hpp"
#include "data/TFCustomKill.hpp"
#include "data/TFTeam.hpp"
#include "data/TFDetailWeapon.hpp"
#include "data/TFWeapon.hpp"
#include "net/worldstate/WorldState.hpp"

#include <map>
#include <queue>
#include <vector>

class RandomCritListener final : IWorldEventListener
{
public:
	RandomCritListener(const std::shared_ptr<WorldState>& world);
	~RandomCritListener();

protected:
	void EntityCreated(const std::shared_ptr<Entity>& ent) override;
	void GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event) override;
	void TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta) override;
	void TempEntityCreated(const std::shared_ptr<TempEntity>& tempEnt) override;
	void PostStringTableCreate(const std::shared_ptr<StringTable>& table) override;

private:
	enum class CritType
	{
		Invalid,		// Not a kill, or some other failure

		NonCrit,
		NonRandomCrit,	// Record as normal kill
		MaybeRandomCrit,	// Record as unknown
		RandomCrit,	// Record as random crit kill
	};

	uint_fast16_t m_SyringeGunModelIndex = std::numeric_limits<decltype(m_SyringeGunModelIndex)>::max();
	void ModelPrecacheUpdate(const std::shared_ptr<const StringTable>& table);

	void OnPlayerHurt(const WorldState& world, const std::shared_ptr<GameEvent>& event);
	void OnPlayerDeath(const WorldState& world, const std::shared_ptr<GameEvent>& event);
	void OnPlayerSpawn(const WorldState& world, const GameEvent& event);
	void OnChargeDeployed(const WorldState& world, const GameEvent& event);

	void OnRocketFired(const std::shared_ptr<Entity>& ent);
	void OnPipeStickyFired(const std::shared_ptr<Entity>& ent);
	void OnArrowFired(const std::shared_ptr<Entity>& arrow);
	void OnFlareFired(const std::shared_ptr<Entity>& flare);
	void OnOrnamentFired(const std::shared_ptr<Entity>& ornament);
	void OnHealingBoltFired(const std::shared_ptr<Entity>& bolt);
	void OnSandmanBallFired(const std::shared_ptr<Entity>& ball);

	void OnProjectileFired(const std::shared_ptr<Entity>& proj, const std::string_view& hLauncherName, const std::string_view& vOriginName);
	void OnProjectileRemoved(const std::shared_ptr<Entity>& proj);

	static bool IsCritBoosted(const Entity& player);

	static bool HasCond(const Entity& ent, TFCond cond);
	static bool IsHitscan(TFDetailWeapon weapon);
	static bool HasRandomCrits(TFDetailWeapon weapon);

	static bool AreTeamsOpposite(TFTeam t1, TFTeam t2);

	void RecordMedigunUsage(uint_fast16_t medicUID, uint_fast32_t tick);

	void CheckForMatchingEvents(const WorldState& world);

	static Vector GetPlayerPosition(const Entity& player);

	struct Projectile
	{
		std::weak_ptr<Entity> m_Proj;
		std::weak_ptr<Entity> m_Player;
		std::string_view m_VecOriginPropName;
		uint_fast16_t m_PlayerUID = std::numeric_limits<decltype(m_PlayerUID)>::max();
		CritType m_Crit;
	};
	std::forward_list<Projectile> m_Projectiles;
	std::vector<std::pair<uint_fast32_t, std::shared_ptr<Entity>>> m_RemovedProjectiles;

	struct SyringeProjectile
	{
		uint_fast32_t m_Tick;
		std::shared_ptr<const TempEntity> m_Spawn;
	};
	std::forward_list<SyringeProjectile> m_SyringeProjectiles;

	bool m_HurtEventThisTick = false;
	bool m_DeathEventThisTick = false;

	struct PlayerHurtEvent
	{
		uint_fast32_t m_Tick;
		uint_fast16_t m_Damage;
		TFWeapon m_WeaponID;
		uint_fast16_t m_VictimUID;
		uint_fast16_t m_AttackerUID;
		bool m_Crit;
		bool m_Minicrit;
		TFCustomKill m_Custom;

		std::shared_ptr<GameEvent> m_FullEvent;
	};
	std::vector<PlayerHurtEvent> m_HurtEvents;

	struct PlayerDeathEvent
	{
		uint_fast32_t m_Tick;
		DamageBits m_DamageBits;
		TFDetailWeapon m_Weapon;
		TFWeapon m_WeaponID;
		uint_fast16_t m_VictimUID;
		uint_fast16_t m_AttackerUID;
		int_fast16_t m_AssisterUID;

		std::shared_ptr<GameEvent> m_FullEvent;
	};
	std::vector<PlayerDeathEvent> m_DeathEvents;

	void OnPlayerDeath(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death);
	CritType WasRandomCritKill(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death);

	CritType OnHitscanCritKill(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death);
	CritType OnProjectileCritKill(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death);

	//bool IsSituationalCrit()

	struct PlayerInfo
	{
		TFClassType m_Class = TFClassType::Unknown;
		TFTeam m_Team = TFTeam::Unassigned;
		size_t m_LastKritzChargeTick = 0;
	};
	std::map<uint_fast16_t, PlayerInfo> m_PlayerInfos;

	struct DelayedEvent
	{
		std::shared_ptr<WorldState> m_World;
		std::shared_ptr<const GameEvent> m_Event;
		uint_fast32_t m_ActualTick;
		uint_fast32_t m_DelayUntilTick;
	};
	std::queue<DelayedEvent> m_DelayedEvents;

	void OnGameRulesEntCreated(Entity& ent);
	void OnGameRulesEntDeleted(const std::shared_ptr<Entity>& ent);
	const RoundState* m_RoundState = nullptr;
	bool IsRoundActive() const;

	size_t m_TotalRandomKills = 0;
	size_t m_TotalNonRandomKills = 0;
	size_t m_TotalNonCritKills = 0;
	size_t m_TotalUnknownKills = 0;

	std::weak_ptr<WorldState> m_World;
};