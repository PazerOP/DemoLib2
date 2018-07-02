#include "RandomCritListener.hpp"

#include "net/data/GameEvent.hpp"
#include "net/data/KnownStringTable.hpp"
#include "net/data/UserInfo.hpp"
#include "net/entities/Entity.hpp"
#include "net/entities/TempEntity.hpp"

#include <bitset>
#include <iomanip>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

struct color_for
{
	constexpr color_for(TFTeam team = TFTeam::Unassigned) : m_Team(team) {}

	TFTeam m_Team;
};

static std::ostream& operator<<(std::ostream& os, const color_for& teamColor)
{
	if (teamColor.m_Team == TFTeam::Red)
		os << cc::bg::dark::red << cc::fg::black;
	else if (teamColor.m_Team == TFTeam::Blue)
		os << cc::bg::blue << cc::fg::black;

	return os;
}

static void OnPlayerCondBitsChanged(SendProp& prop)
{
	uint_fast8_t startBit = 0;
	if (prop.GetFullName() == "DT_TFPlayerConditionListExclusive._condition_bits"sv || prop.GetFullName() == "DT_TFPlayerShared.m_nPlayerCond"sv)
		startBit = 0;
	else if (prop.GetFullName() == "DT_TFPlayerShared.m_nPlayerCondEx"sv)
		startBit = 32;
	else if (prop.GetFullName() == "DT_TFPlayerShared.m_nPlayerCondEx2"sv)
		startBit = 64;
	else if (prop.GetFullName() == "DT_TFPlayerShared.m_nPlayerCondEx3"sv)
		startBit = 96;
	else
	{
		assert(!"Unknown tfcond prop");
	}

	std::bitset<TFCond::TFCond_COUNT> tfconds;

	for (uint_fast8_t i = 0; i < std::min(32, TFCond_COUNT - startBit); i++)
		tfconds.set(i + startBit, prop.Get<uint32_t>() & (1 << i));

	for (uint_fast8_t i = 0; i < TFCond_COUNT; i++)
	{
		switch (i)
		{
			case TFCond_Kritzkrieged:
			case TFCond_HalloweenCritCandy:
			case TFCond_CritCanteen:
			case TFCond_CritOnFirstBlood:
			case TFCond_CritOnWin:
			case TFCond_CritOnFlagCapture:
			case TFCond_CritOnKill:
			case TFCond_CritMmmph:
			case TFCond_CritOnDamage:
			case TFCond_RuneCrit:
				break;

			default:
				tfconds.reset(i);
		}
	}

	tfconds.set(TFCond_BlastJumping, false);
	tfconds.set(TFCond_Overhealed, false);
	tfconds.set(TFCond_SpawnOutline, false);

	if (!tfconds.any())
		return;

	UserInfo info;
	bool result = prop.GetEntity()->GetWorld()->GetUserByEntindex(static_cast<Entity*>(prop.GetEntity())->GetIndex(), info);
	assert(result);

	cc::out << prop.GetFullName() << " for " << info.m_Name << " updated: \n\t";

	bool previous = false;
	for (uint_fast8_t i = 0; i < std::min(32, TFCond_COUNT - startBit); i++)
	{
		if (prop.Get<uint32_t>() & (1 << i))
		{
			if (previous)
				cc::out << ' ';

			cc::out << TFCond(i + startBit);
			previous = true;
		}
	}

	cc::out << std::endl;
}

RandomCritListener::RandomCritListener(const std::shared_ptr<WorldState>& world) : m_World(world)
{
	world->m_Events.AddEventListener(this);
}

RandomCritListener::~RandomCritListener()
{
	if (auto locked = m_World.lock())
		locked->m_Events.RemoveEventListener(this);
}

void RandomCritListener::EntityCreated(const std::shared_ptr<Entity>& ent)
{
	if (ent->Is("DT_TFProjectile_Rocket"sv, false))
		OnRocketFired(ent);
	else if (ent->Is("DT_TFProjectile_Pipebomb"sv, false))
		OnPipeStickyFired(ent);
	else if (ent->Is("DT_TFProjectile_Arrow"sv, false))
		OnArrowFired(ent);
	else if (ent->Is("DT_TFProjectile_Flare"sv, false))
		OnFlareFired(ent);
	else if (ent->Is("DT_TFProjectileBall_Ornament"sv, false))
		OnOrnamentFired(ent);
	else if (ent->Is("DT_TFProjectile_HealingBolt"sv, false))
		OnHealingBoltFired(ent);
	else if (ent->Is("DT_TFProjectile_StunBall"sv, false))
		OnSandmanBallFired(ent);
	else if (ent->Is("DT_TFProjectile_Jar"sv, false) ||
		ent->Is("DT_TFProjectile_JarMilk"sv, false) ||
		ent->Is("DT_TFProjectile_JarGas"sv, false) ||
		ent->Is("DT_TFProjectile_SentryRocket"sv, false) ||
		ent->Is("DT_TFProjectile_BallOfFire"sv, false) ||
		ent->Is("DT_TFProjectile_EnergyRing"sv, false) ||
		ent->Is("DT_TFProjectile_EnergyBall"sv, false) ||
		ent->Is("DT_TFProjectile_Cleaver"sv, false))
	{
		// We don't track these since they do no damage or can't be random crits
		return;
	}
	else if (ent->Is("DT_BaseProjectile"))
		cc::out << "Unknown projectile: " << ent->GetNetworkTable()->GetName() << std::endl;
	else if (ent->Is("DT_TFGameRulesProxy"sv, false))
		OnGameRulesEntCreated(*ent);
}

void RandomCritListener::GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event)
{
	if (event->Is("player_hurt"))
		OnPlayerHurt(*world, event);
	else if (event->Is("player_death"))
		OnPlayerDeath(*world, event);
	else if (event->Is("player_spawn"))
		OnPlayerSpawn(*world, *event);
	else if (event->Is("player_chargedeployed"))
		OnChargeDeployed(*world, *event);
}

void RandomCritListener::TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta)
{
	m_HurtEvents.clear();
	m_DeathEvents.clear();

	const auto& last = std::remove_if(m_RemovedProjectiles.begin(), m_RemovedProjectiles.end(),
		[&world](const auto& ent)
	{
		return (world->m_Tick - ent.first) > (66 * 5);
	});
	m_RemovedProjectiles.erase(last, m_RemovedProjectiles.end());
}

void RandomCritListener::TempEntityCreated(const std::shared_ptr<TempEntity>& tempEnt)
{
	if (!tempEnt->Is("DT_TEEffectDispatch"))
		return;

	if ((*tempEnt)["DT_EffectData.m_nMaterial"sv].Get<uint32_t>() != m_SyringeGunModelIndex)
		return;	// Only care about syringe gun projectiles

	auto origin = tempEnt->FindPropVec3("DT_EffectData.m_vOrigin"sv, vec3_nan);
	//cc::out << "Syringe gun projectile @ " << origin << std::endl;

	auto& newEntry = m_SyringeProjectiles.emplace_front();
	newEntry.m_Tick = tempEnt->GetWorld()->m_Tick;
	newEntry.m_Spawn = tempEnt;
}

void RandomCritListener::PostStringTableCreate(const std::shared_ptr<StringTable>& table)
{
	if (table->GetName() != "modelprecache"sv)
		return;

	ModelPrecacheUpdate(table);
	table->OnUpdate += std::bind(&RandomCritListener::ModelPrecacheUpdate, this, std::placeholders::_1);
}

void RandomCritListener::ModelPrecacheUpdate(const std::shared_ptr<const StringTable>& table)
{
	assert(std::numeric_limits<decltype(m_SyringeGunModelIndex)>::max() >= table->GetMaxEntries());

	uint_fast16_t index = 0;
	for (const auto& entry : *table)
	{
		if (entry.GetString() == "models/weapons/w_models/w_syringe_proj.mdl"sv)
		{
			m_SyringeGunModelIndex = index;
			return;
		}

		index++;
	}

	m_SyringeGunModelIndex = std::numeric_limits<decltype(m_SyringeGunModelIndex)>::max();
	assert(!"Unable to find syringe gun projectile model in modelprecache string table");
}

void RandomCritListener::OnPlayerHurt(const WorldState& world, const std::shared_ptr<GameEvent>& event)
{
	m_HurtEventThisTick = true;

	if (!event->GetBool("crit") || event->GetBool("minicrit"))
		return;	// Only care about full crits

	if (auto health = event->GetInt("health"))
	{
		assert(health > 0);
		return;	// Only care about kill shots
	}

	auto& newHurtEvent = m_HurtEvents.emplace_back();
	newHurtEvent.m_Tick = world.m_Tick;
	newHurtEvent.m_Damage = event->GetInt("damageamount");
	newHurtEvent.m_WeaponID = (TFWeapon)event->GetInt("weaponid");
	newHurtEvent.m_VictimUID = event->GetInt("userid");
	newHurtEvent.m_AttackerUID = event->GetInt("attacker");
	newHurtEvent.m_Crit = event->GetBool("crit");
	newHurtEvent.m_Minicrit = event->GetBool("minicrit");
	newHurtEvent.m_Custom = (TFCustomKill)event->GetInt("custom");
	newHurtEvent.m_FullEvent = event;

	CheckForMatchingEvents(world);
}

void RandomCritListener::OnPlayerDeath(const WorldState& world, const std::shared_ptr<GameEvent>& event)
{
	const DamageBits damageBits = (DamageBits)event->GetInt("damagebits");
	if (!(damageBits & DMG_CRITICAL))
	{
		m_TotalNonCritKills++;
		return;
	}

	auto& newDeathEvent = m_DeathEvents.emplace_back();
	newDeathEvent.m_Tick = world.m_Tick;
	newDeathEvent.m_DamageBits = (DamageBits)event->GetInt("damagebits");
	newDeathEvent.m_WeaponID = (TFWeapon)event->GetInt("weaponid");
	newDeathEvent.m_Weapon = FindTFDetailWeapon(event->GetString("weapon_logclassname"), newDeathEvent.m_WeaponID);
	newDeathEvent.m_VictimUID = event->GetInt("userid");
	newDeathEvent.m_AttackerUID = event->GetInt("attacker");
	newDeathEvent.m_AssisterUID = event->GetInt("assister");
	newDeathEvent.m_FullEvent = event;

	CheckForMatchingEvents(world);
}

void RandomCritListener::OnPlayerDeath(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death)
{
	// Ignore deaths when round is not active
	if (!IsRoundActive())
		return;

	bool wasRandom = true;
	const auto critType = WasRandomCritKill(world, hurt, death);
	switch (critType)
	{
		case CritType::RandomCrit:
			m_TotalRandomKills++;
			break;
		case CritType::MaybeRandomCrit:
			m_TotalUnknownKills++;
			break;
		case CritType::NonRandomCrit:
			m_TotalNonRandomKills++;
			wasRandom = false;
			break;
		case CritType::NonCrit:
			m_TotalNonCritKills++;
			wasRandom = false;
			break;

		case CritType::Invalid:
			return;
	}

	if (wasRandom)
	{
		UserInfo victim, attacker;
		uint_fast8_t victimEntindex, attackerEntindex;
		if (!world.FindUserByID(death.m_VictimUID, &victim, &victimEntindex))
		{
			cc::out << cc::fg::red << "Unable to find victim with userid " << death.m_VictimUID << cc::endl;
			return;
		}
		if (!world.FindUserByID(death.m_AttackerUID, &attacker, &attackerEntindex))
		{
			cc::out << cc::fg::red << "Unable to find attacker with userid " << death.m_AttackerUID << cc::endl;
			return;
		}

		color_for victimColor, attackerColor;
		if (auto victimPlayer = world.m_Entities[victimEntindex])
			victimColor = color_for((*victimPlayer)["DT_BaseEntity.m_iTeamNum"sv].Get<TFTeam>());
		if (auto attackerPlayer = world.m_Entities[attackerEntindex])
			attackerColor = color_for((*attackerPlayer)["DT_BaseEntity.m_iTeamNum"sv].Get<TFTeam>());

		cc::out
			<< attackerColor << attacker.m_Name << cc::reset
			<< " killed " << victimColor << victim.m_Name << cc::reset
			<< " with a ";

		switch (critType)
		{
			case CritType::RandomCrit:
				cc::out << cc::fg::red << "random";
				break;
			case CritType::MaybeRandomCrit:
				cc::out << cc::fg::yellow << "maybe-random";
				break;
			case CritType::NonRandomCrit:
				cc::out << cc::fg::green << "non-random";
				break;
			default:
				cc::out << cc::fg::cyan << cc::bold << "UNKNOWN";
				break;
		}

		cc::out << cc::reset << " crit from a " << death.m_Weapon
			<< ", dealing " << hurt.m_Damage << " damage." << cc::endl;

		const auto totalKills = m_TotalRandomKills + m_TotalNonRandomKills + m_TotalNonCritKills + m_TotalUnknownKills;

		cc::out << cc::fg::grey
			<< "Randoms: " << std::setprecision(1) << std::fixed << std::setw(5) << std::left << ((float)m_TotalRandomKills / totalKills) * 100
			<< " Non-randoms: " << std::setprecision(1) << std::fixed << std::setw(5) << std::left << ((float)m_TotalNonRandomKills / totalKills) * 100
			<< " Maybe-randoms: " << std::setprecision(1) << std::fixed << std::setw(5) << std::left << ((float)m_TotalUnknownKills / totalKills) * 100
			<< " Non-crits: " << std::setprecision(1) << std::fixed << std::setw(5) << std::left << ((float)m_TotalNonCritKills / totalKills) * 100
			<< cc::endl;
	}
}

void RandomCritListener::OnPlayerSpawn(const WorldState& world, const GameEvent& event)
{
	// Clean up playerinfos from users no longer on the server
	for (auto iter = m_PlayerInfos.begin(); iter != m_PlayerInfos.end(); ++iter)
	{
		if (!world.FindUserByID(iter->first, nullptr))
		{
			iter = m_PlayerInfos.erase(iter);
			if (iter == m_PlayerInfos.end())
				break;
		}
	}

	auto& spawned = m_PlayerInfos[event.GetInt("userid")];
	spawned.m_Class = (TFClassType)event.GetInt("class");
	spawned.m_Team = (TFTeam)event.GetInt("team");
}

void RandomCritListener::OnChargeDeployed(const WorldState& world, const GameEvent& event)
{
	const auto medicUID = event.GetInt("userid"sv);
	uint_fast8_t medicEntindex;
	if (!world.FindUserByID(medicUID, nullptr, &medicEntindex))
	{
		cc::out << "Unable to find medic with userid " << medicUID << std::endl;
		return;
	}

	auto medicEnt = world.m_Entities[medicEntindex];
	if (!medicEnt || !medicEnt->IsInPVS())
		return RecordMedigunUsage(medicUID, world.m_Tick);	// Unknown medic (and medigun)

	const auto& medigunEnt = (*medicEnt)["m_hMyWeapons.001"sv].Get<Entity>();
	if (!medigunEnt || !medigunEnt->IsInPVS())
		return RecordMedigunUsage(medicUID, world.m_Tick);	// Unknown medigun
	else if (auto itemID = (*medigunEnt)["DT_ScriptCreatedItem.m_iItemDefinitionIndex"sv].Get<int>(); itemID == 35)
		return RecordMedigunUsage(medicUID, world.m_Tick);	// Actually a kritzkrieg

	// If we get here, we *KNOW* that the pop wasn't with a kritzkrieg
}

static void CritStateChanged(SendProp& prop)
{
	assert(!"Crit state should never change after creation!");
}

void RandomCritListener::OnRocketFired(const std::shared_ptr<Entity>& rocket)
{
	if (auto& crit = (*rocket)["DT_TFProjectile_Rocket.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(rocket, "DT_TFBaseRocket.m_hLauncher"sv, "DT_TFBaseRocket.m_vecOrigin"sv);
}

void RandomCritListener::OnPipeStickyFired(const std::shared_ptr<Entity>& pipeOrSticky)
{
	if (auto& crit = (*pipeOrSticky)["DT_TFWeaponBaseGrenadeProj.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(pipeOrSticky, "DT_TFProjectile_Pipebomb.m_hLauncher"sv, "DT_TFWeaponBaseGrenadeProj.m_vecOrigin"sv);
}

void RandomCritListener::OnArrowFired(const std::shared_ptr<Entity>& arrow)
{
	if (auto& crit = (*arrow)["DT_TFProjectile_Arrow.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(arrow, "DT_TFBaseRocket.m_hLauncher"sv, "DT_TFBaseRocket.m_vecOrigin"sv);
}

void RandomCritListener::OnFlareFired(const std::shared_ptr<Entity>& flare)
{
	if (auto& crit = (*flare)["DT_TFProjectile_Flare.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(flare, "DT_TFBaseRocket.m_hLauncher"sv, "DT_TFBaseRocket.m_vecOrigin"sv);
}

void RandomCritListener::OnOrnamentFired(const std::shared_ptr<Entity>& ornament)
{
	if (auto& crit = (*ornament)["DT_TFWeaponBaseGrenadeProj.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(ornament, "DT_TFProjectile_Pipebomb.m_hLauncher"sv, "DT_TFWeaponBaseGrenadeProj.m_vecOrigin"sv);
}

void RandomCritListener::OnHealingBoltFired(const std::shared_ptr<Entity>& bolt)
{
	if (auto& crit = (*bolt)["DT_TFProjectile_Arrow.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(bolt, "DT_TFBaseRocket.m_hLauncher"sv, "DT_TFBaseRocket.m_vecOrigin"sv);
}

void RandomCritListener::OnSandmanBallFired(const std::shared_ptr<Entity>& ball)
{
	if (auto& crit = (*ball)["DT_TFWeaponBaseGrenadeProj.m_bCritical"sv]; !crit.Get<bool>())
	{
		crit.OnValueChanged += std::bind(&CritStateChanged, std::placeholders::_1);
		return;	// Don't care about non-crits
	}

	OnProjectileFired(ball, "DT_TFProjectile_Pipebomb.m_hLauncher"sv, "DT_TFWeaponBaseGrenadeProj.m_vecOrigin"sv);
}

static void OnAnimTimeChanged(SendProp& prop)
{
	cc::out << "anim time changed: " << prop.Get<int>() << std::endl;
}

static void OnSimulationTimeChanged(SendProp& prop)
{
	cc::out << "simulation time changed: " << prop.Get<int>() << std::endl;
}

void RandomCritListener::OnProjectileFired(const std::shared_ptr<Entity>& proj, const std::string_view& hLauncherName, const std::string_view& vOriginName)
{
	// Don't track projectiles when the game isn't running
	if (!IsRoundActive())
		return;

	const auto& launcher = (*proj)[hLauncherName].Get<Entity>();
	if (!launcher)
		cc::out << "Unable to properly track newly created projectile: " << hLauncherName << " referenced unknown or invalid entity" << std::endl;

	//(*proj)["DT_AnimTimeMustBeFirst.m_flAnimTime"sv].OnValueChanged += std::bind(&OnAnimTimeChanged, std::placeholders::_1);
	//(*proj)["DT_BaseEntity.m_flSimulationTime"sv].OnValueChanged += std::bind(&OnSimulationTimeChanged, std::placeholders::_1);

	const auto& launcherPlayer = launcher ? (*launcher)["DT_BaseCombatWeapon.m_hOwner"].Get<Entity>() : nullptr;

	Projectile& newProjEntry = m_Projectiles.emplace_front();
	newProjEntry.m_Proj = proj;
	newProjEntry.m_Player = launcherPlayer;
	newProjEntry.m_VecOriginPropName = vOriginName;

	proj->OnDeleted += std::bind(&RandomCritListener::OnProjectileRemoved, this, std::placeholders::_1);

	if (!launcherPlayer || !launcherPlayer->IsInPVS())
	{
		cc::out << cc::bg::dark::yellow << cc::fg::black << "Unable to properly track newly created projectile: launcher player null or not in PVS" << cc::endl;
		newProjEntry.m_Crit = CritType::MaybeRandomCrit;
	}
	else
	{
		auto playerPos = GetPlayerPosition(*launcherPlayer);
		auto rkPos = proj->FindPropVec3(vOriginName, vec3_nan);

		Vector velocity = vec3_nan;
		if (auto prop = proj->FindProperty("DT_TFWeaponBaseGrenadeProj.m_vInitialVelocity"sv))
			velocity = prop->Get<Vector>();
		else if (prop = proj->FindProperty("DT_TFBaseRocket.m_vInitialVelocity"sv))
			velocity = prop->Get<Vector>();


		rkPos -= velocity * (1.0f / 66);

		playerPos.z = rkPos.z;

		const auto dist = playerPos.DistTo(rkPos);
		if (dist < 50)
			newProjEntry.m_Crit = IsCritBoosted(*launcherPlayer) ? CritType::NonRandomCrit : CritType::RandomCrit;
		else
		{
			const auto speed = velocity.Length();
			newProjEntry.m_Crit = CritType::MaybeRandomCrit;
		}

		if (UserInfo attackerInfo; launcherPlayer && proj->GetWorld()->GetUserByEntindex(launcherPlayer->GetIndex(), attackerInfo))
		{
			newProjEntry.m_PlayerUID = attackerInfo.m_UserID;

#if 1
			cc::out << "Tracking ";

			if (newProjEntry.m_Crit == CritType::NonRandomCrit)
				cc::out << cc::fg::green << "non-random";
			else if (newProjEntry.m_Crit == CritType::MaybeRandomCrit)
				cc::out << cc::fg::yellow << "maybe-random";
			else if (newProjEntry.m_Crit == CritType::RandomCrit)
				cc::out << cc::fg::red << "random";
			else
				cc::out << cc::fg::cyan << cc::bold << "UNKNOWN";

			cc::out << cc::reset << " crit " << proj->GetNetworkTable()->GetName() << " from "
				<< color_for((*launcherPlayer)["DT_BaseEntity.m_iTeamNum"sv].Get<TFTeam>()) << attackerInfo.m_Name << cc::endl;
#endif
		}
		else
		{
			cc::out << cc::bg::dark::yellow << cc::fg::black << "Unable to properly track newly created projectile: couldn't find attacker UserInfo" << cc::endl;
		}
	}
}

void RandomCritListener::OnProjectileRemoved(const std::shared_ptr<Entity>& proj)
{
	m_RemovedProjectiles.emplace_back(proj->GetWorld()->m_Tick, proj);    // Keep a shared_ptr to this entity for the rest of this tick
}

RandomCritListener::CritType RandomCritListener::OnHitscanCritKill(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death)
{
	UserInfo attackerInfo, victimInfo;
	uint_fast8_t attackerEntindex, victimEntIndex;
	if (!world.FindUserByID(death.m_AttackerUID, &attackerInfo, &attackerEntindex))
	{
		cc::out << "Couldn't find attacker with userid " << death.m_AttackerUID << std::endl;
		return CritType::Invalid;	// Couldn't find attacker
	}
	if (!world.FindUserByID(death.m_VictimUID, &victimInfo, &victimEntIndex))
	{
		cc::out << "Couldn't find victim with userid " << death.m_VictimUID << std::endl;
		return CritType::Invalid;	// Couldn't find victim
	}

	const auto& attackerEnt = world.m_Entities[attackerEntindex];
	if (!attackerEnt || !attackerEnt->IsInPVS())
	{
		// Check the last time a medigun charge was used on the other team
		const auto victimTeam = m_PlayerInfos[death.m_VictimUID].m_Team;

		size_t latestCharge = 0;
		for (const auto& player : m_PlayerInfos)
		{
			if (AreTeamsOpposite(player.second.m_Team, victimTeam))
				latestCharge = std::max(latestCharge, player.second.m_LastKritzChargeTick);
		}

		if ((world.m_Tick - latestCharge) < (66 * 7))
			return CritType::MaybeRandomCrit;	// If we're here, there is *potentially* a kritzkrieg active on the other team
		else
			return CritType::RandomCrit;		// No way this could be kritz
	}

	if (IsCritBoosted(*attackerEnt))
		return CritType::NonRandomCrit;	// Ignore crit boosted weapons, we're looking for random crits only

	return CritType::RandomCrit;
}

RandomCritListener::CritType RandomCritListener::OnProjectileCritKill(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death)
{
	UserInfo victimInfo;
	uint_fast8_t victimEntindex;
	if (!world.FindUserByID(death.m_VictimUID, &victimInfo, &victimEntindex))
	{
		cc::out << "Couldn't find victim with userid " << death.m_VictimUID << std::endl;
		return CritType::Invalid;
	}

	// Remove expired
	m_Projectiles.remove_if([](const Projectile& proj) { return proj.m_Proj.expired(); });

	Vector victimPos = vec3_nan;
	{
		const auto& victimEnt = world.m_Entities[victimEntindex];
		if (!victimEnt || !victimEnt->IsInPVS())
		{
			cc::out << "OnProjectileCritKill(): Victim " << victimInfo.m_Name << " was ";
			if (victimEnt)
				cc::out << "outside PVS";
			else
				cc::out << "null";

			cc::out << std::endl;

			return CritType::MaybeRandomCrit;	// Unable to find our victim
		}

		victimPos = GetPlayerPosition(*victimEnt);
	}

	struct test
	{
		float dist;
		uint_fast32_t tickDelta;
		const Projectile* proj;

		bool operator<(const test& rhs) const { return dist < rhs.dist; }
	};
	std::vector<test> testSorted;

	for (const auto& proj : m_Projectiles)
	{
		assert(proj.m_Crit != CritType::NonCrit);
		assert(proj.m_Crit != CritType::Invalid);
		if (proj.m_Crit == CritType::NonRandomCrit)
			continue;

		if (proj.m_PlayerUID != death.m_AttackerUID)
			continue;

		auto lockedProj = proj.m_Proj.lock();
		if (!lockedProj)
			continue;

		const auto tickDelta = lockedProj->GetWorld()->m_Tick - lockedProj->GetLastPVSStateChangeTick();
		assert(lockedProj->IsInPVS() || tickDelta <= 66 * 5);

		auto& newTest = testSorted.emplace_back();
		newTest.proj = &proj;

		newTest.tickDelta = lockedProj->IsInPVS() ? 0 : tickDelta;

		const Vector& currentPos = (*lockedProj)[proj.m_VecOriginPropName].Get<Vector>();
		newTest.dist = currentPos.DistTo(victimPos);
	}

	std::sort(testSorted.begin(), testSorted.end());

	bool atLeastOne = !testSorted.empty();
	if (testSorted.empty()/* || testSorted.front().dist > 500*/)
	{
		const auto distAway = world.m_DemoViewPos.DistTo(victimPos);
		cc::out << cc::fg::yellow << "Unable to find projectile that killed " << victimInfo.m_Name << " (" << distAway << " units away)" << cc::endl;
		return CritType::MaybeRandomCrit;    // Indeterminate
	}

	const auto victimTeam = (*world.m_Entities[victimEntindex])["DT_BaseEntity.m_iTeamNum"sv].Get<TFTeam>();
	if (victimTeam == TFTeam::Red)
		cc::out << cc::fg::dark::red;
	else if (victimTeam == TFTeam::Blue)
		cc::out << cc::fg::dark::blue;

	cc::out << victimInfo.m_Name << cc::reset << " killed by crit projectile that was " << testSorted.front().dist << " units and " << testSorted.front().tickDelta << " ticks away?" << cc::endl;

	return testSorted.front().proj->m_Crit;
}

void RandomCritListener::OnGameRulesEntCreated(Entity& ent)
{
	cc::out << cc::fg::green << "Game rules entity created" << cc::endl;
	m_RoundState = &(ent["DT_TeamplayRoundBasedRules.m_iRoundState"sv].Get<RoundState>());

	ent.OnDeleted += std::bind(&RandomCritListener::OnGameRulesEntDeleted, this, std::placeholders::_1);
	//m_bInSetup = &(ent["DT_TeamplayRoundBasedRules.m_bInSetup"sv].Get<int>());
}

void RandomCritListener::OnGameRulesEntDeleted(const std::shared_ptr<Entity>& ent)
{
	m_RoundState = nullptr;
}

RandomCritListener::CritType RandomCritListener::WasRandomCritKill(const WorldState& world, const PlayerHurtEvent& hurt, const PlayerDeathEvent& death)
{
	if (!hurt.m_Crit || hurt.m_Minicrit)
		return CritType::NonCrit;

	if (!HasRandomCrits(death.m_Weapon))
		return CritType::NonRandomCrit;

	if (IsHitscan(death.m_Weapon))
		return OnHitscanCritKill(world, hurt, death);
	else
		return OnProjectileCritKill(world, hurt, death);
}

bool RandomCritListener::IsCritBoosted(const Entity& player)
{
	if (HasCond(player, TFCond_Kritzkrieged))
		return true;
	if (HasCond(player, TFCond_HalloweenCritCandy))
		return true;
	if (HasCond(player, TFCond_CritCanteen))
		return true;
	if (HasCond(player, TFCond_CritOnFirstBlood))
		return true;
	if (HasCond(player, TFCond_CritOnWin))
		return true;
	if (HasCond(player, TFCond_CritOnFlagCapture))
		return true;
	if (HasCond(player, TFCond_CritOnKill))
		return true;
	if (HasCond(player, TFCond_CritMmmph))
		return true;
	if (HasCond(player, TFCond_CritOnDamage))
		return true;
	if (HasCond(player, TFCond_RuneCrit))
		return true;

	return false;
}

bool RandomCritListener::HasCond(const Entity& ent, TFCond cond)
{
	if (cond < 32)
	{
		uint32_t playerCond = ent["DT_TFPlayerShared.m_nPlayerCond"sv].Get<uint32_t>();
		uint32_t condBits = ent["DT_TFPlayerConditionListExclusive._condition_bits"sv].Get<uint32_t>();

		return (playerCond | condBits) & (1 << cond);
	}
	else if (cond < 64)
	{
		uint32_t playerCondEx = ent["DT_TFPlayerShared.m_nPlayerCondEx"sv].Get<uint32_t>();

		return playerCondEx & (1 << (cond - 32));
	}
	else if (cond < 96)
	{
		uint32_t playerCondEx = ent["DT_TFPlayerShared.m_nPlayerCondEx2"sv].Get<uint32_t>();

		return playerCondEx & (1 << (cond - 64));
	}
	else if (cond < 128)
	{
		uint32_t playerCondEx = ent["DT_TFPlayerShared.m_nPlayerCondEx3"sv].Get<uint32_t>();

		return playerCondEx & (1 << (cond - 96));
	}
	else
		throw std::domain_error("Invalid TFCond");
}

#pragma warning(push)
//#pragma warning(1 : 4061)
//#pragma warning(1 : 4062)
#pragma warning(error : 4061)
#pragma warning(error : 4062)
bool RandomCritListener::IsHitscan(TFDetailWeapon weapon)
{
	switch (weapon)
	{
		case TFDetailWeapon::Airstrike:
		case TFDetailWeapon::Backburner:
		case TFDetailWeapon::BeggarsBazooka:
		case TFDetailWeapon::BlackBox:
		case TFDetailWeapon::Blutsauger:
		case TFDetailWeapon::CowMangler:
		case TFDetailWeapon::CrusadersCrossbow:
		case TFDetailWeapon::Degreaser:
		case TFDetailWeapon::Detonator:
		case TFDetailWeapon::DirectHit:
		case TFDetailWeapon::DragonsFury:
		case TFDetailWeapon::Flamethrower:
		case TFDetailWeapon::FlareGun:
		case TFDetailWeapon::GrenadeLauncher:
		case TFDetailWeapon::Huntsman:
		case TFDetailWeapon::IronBomber:
		case TFDetailWeapon::LibertyLauncher:
		case TFDetailWeapon::LochNLoad:
		case TFDetailWeapon::LooseCannon:
		case TFDetailWeapon::Original:
		case TFDetailWeapon::Overdose:
		case TFDetailWeapon::Phlogistinator:
		case TFDetailWeapon::Pomson:
		case TFDetailWeapon::QuickiebombLauncher:
		case TFDetailWeapon::ReflectHuntsman:
		case TFDetailWeapon::ReflectPill:
		case TFDetailWeapon::ReflectRocket:
		case TFDetailWeapon::RescueRanger:
		case TFDetailWeapon::RocketLauncher:
		case TFDetailWeapon::ScoutBall:
		case TFDetailWeapon::ScorchShot:
		case TFDetailWeapon::ScottishResistance:
		case TFDetailWeapon::SentryRocket:
		case TFDetailWeapon::StickyLauncher:
		case TFDetailWeapon::SyringeGun:
			return false;

		case TFDetailWeapon::Atomizer:
		case TFDetailWeapon::BabyFacesBlaster:
		case TFDetailWeapon::BackScatter:
		case TFDetailWeapon::BackScratcher:
		case TFDetailWeapon::Bat:
		case TFDetailWeapon::BazaarBargain:
		case TFDetailWeapon::BostonBasher:
		case TFDetailWeapon::Bottle:
		case TFDetailWeapon::BrassBeast:
		case TFDetailWeapon::Claidheamohmor:
		case TFDetailWeapon::ConscientiousObjector:
		case TFDetailWeapon::Diamondback:
		case TFDetailWeapon::DisciplinaryAction:
		case TFDetailWeapon::EscapePlan:
		case TFDetailWeapon::EurekaEffect:
		case TFDetailWeapon::Eyelander:
		case TFDetailWeapon::FamilyBusiness:
		case TFDetailWeapon::FanOWar:
		case TFDetailWeapon::Fists:
		case TFDetailWeapon::FistsOfSteel:
		case TFDetailWeapon::ForceANature:
		case TFDetailWeapon::FrontierJustice:
		case TFDetailWeapon::FreedomStaff:
		case TFDetailWeapon::FryingPan:
		case TFDetailWeapon::GlovesOfRunningUrgently:
		case TFDetailWeapon::Gunslinger:
		case TFDetailWeapon::HalfZatoichi:
		case TFDetailWeapon::HamShank:
		case TFDetailWeapon::HitmansHeatmaker:
		case TFDetailWeapon::Homewrecker:
		case TFDetailWeapon::HotHand:
		case TFDetailWeapon::HuoLongHeater:
		case TFDetailWeapon::Jag:
		case TFDetailWeapon::Knife:
		case TFDetailWeapon::Letranger:
		case TFDetailWeapon::MarketGardener:
		case TFDetailWeapon::Minigun:
		case TFDetailWeapon::Natascha:
		case TFDetailWeapon::NecroSmasher:
		case TFDetailWeapon::PainTrain:
		case TFDetailWeapon::PanicAttack:
		case TFDetailWeapon::Pistol:
		case TFDetailWeapon::PrettyBoysPocketPistol:
		case TFDetailWeapon::PrinnyMachete:
		case TFDetailWeapon::Powerjack:
		case TFDetailWeapon::ReserveShooter:
		case TFDetailWeapon::Revolver:
		case TFDetailWeapon::Sandman:
		case TFDetailWeapon::Scattergun:
		case TFDetailWeapon::ScotsmansSkullcutter:
		case TFDetailWeapon::SentryLevel1:
		case TFDetailWeapon::SentryLevel2:
		case TFDetailWeapon::SentryLevel3:
		case TFDetailWeapon::SentryMini:
		case TFDetailWeapon::Shahanshah:
		case TFDetailWeapon::SharpDresser:
		case TFDetailWeapon::Shortstop:
		case TFDetailWeapon::Shotgun:
		case TFDetailWeapon::SkullBat:
		case TFDetailWeapon::SodaPopper:
		case TFDetailWeapon::SoldierGrenadeTaunt:
		case TFDetailWeapon::SolemnVow:
		case TFDetailWeapon::Telefrag:
		case TFDetailWeapon::Tomislav:
		case TFDetailWeapon::TribalmansShiv:
		case TFDetailWeapon::Ubersaw:
		case TFDetailWeapon::UllapoolCaber:
		case TFDetailWeapon::UnarmedCombat:
		case TFDetailWeapon::WarriorsSpirit:
		case TFDetailWeapon::Widowmaker:
		case TFDetailWeapon::WrapAssassin:
		case TFDetailWeapon::Wrench:
		case TFDetailWeapon::KillingGlovesOfBoxing:
		case TFDetailWeapon::Wrangler:
		case TFDetailWeapon::Amputator:
		case TFDetailWeapon::Bonesaw:
		case TFDetailWeapon::AwperHand:
		case TFDetailWeapon::Bushwacka:
		case TFDetailWeapon::Machina:
		case TFDetailWeapon::CleanersCarbine:
		case TFDetailWeapon::SMG:
		case TFDetailWeapon::SniperRifle:
		case TFDetailWeapon::SydneySleeper:
		case TFDetailWeapon::Classic:
		case TFDetailWeapon::Ambassador:
		case TFDetailWeapon::BigEarner:
		case TFDetailWeapon::Enforcer:
		case TFDetailWeapon::YourEternalReward:
		case TFDetailWeapon::Club:
		case TFDetailWeapon::Kunai:
		case TFDetailWeapon::SpyCicle:
			return true;

		case TFDetailWeapon::Unknown:
		case TFDetailWeapon::Environment:
		//default:
			cc::dbg << "Unknown TFDetailWeapon " << weapon << " in " __FUNCTION__ << std::endl;
			return true;
	}

	throw std::invalid_argument("Out of range TFDetailWeapon");
}

bool RandomCritListener::HasRandomCrits(TFDetailWeapon weapon)
{
	switch (weapon)
	{
		case TFDetailWeapon::Ambassador:
		case TFDetailWeapon::AwperHand:
		case TFDetailWeapon::Backburner:
		case TFDetailWeapon::BazaarBargain:
		case TFDetailWeapon::BigEarner:
		case TFDetailWeapon::Bushwacka:
		case TFDetailWeapon::Classic:
		case TFDetailWeapon::CowMangler:
		case TFDetailWeapon::Diamondback:
		case TFDetailWeapon::Eyelander:
		case TFDetailWeapon::FrontierJustice:
		case TFDetailWeapon::Gunslinger:
		case TFDetailWeapon::HitmansHeatmaker:
		case TFDetailWeapon::Huntsman:
		case TFDetailWeapon::Knife:
		case TFDetailWeapon::Kunai:
		case TFDetailWeapon::Machina:
		case TFDetailWeapon::MarketGardener:
		case TFDetailWeapon::Phlogistinator:
		case TFDetailWeapon::SharpDresser:
		case TFDetailWeapon::SniperRifle:
		case TFDetailWeapon::SoldierGrenadeTaunt:
		case TFDetailWeapon::SpyCicle:
		case TFDetailWeapon::Telefrag:
		case TFDetailWeapon::YourEternalReward:
		case TFDetailWeapon::HalfZatoichi:
		case TFDetailWeapon::ReflectHuntsman:
		case TFDetailWeapon::Claidheamohmor:
		case TFDetailWeapon::DragonsFury:
		case TFDetailWeapon::SentryMini:
		case TFDetailWeapon::SentryLevel1:
		case TFDetailWeapon::SentryLevel2:
		case TFDetailWeapon::SentryLevel3:
		case TFDetailWeapon::SentryRocket:
		case TFDetailWeapon::CleanersCarbine:
		case TFDetailWeapon::UllapoolCaber:
		case TFDetailWeapon::Wrangler:
		case TFDetailWeapon::SydneySleeper:
		case TFDetailWeapon::Enforcer:
			return false;

		case TFDetailWeapon::Airstrike:
		case TFDetailWeapon::Atomizer:
		case TFDetailWeapon::BabyFacesBlaster:
		case TFDetailWeapon::Bat:
		case TFDetailWeapon::BeggarsBazooka:
		case TFDetailWeapon::BlackBox:
		case TFDetailWeapon::BostonBasher:
		case TFDetailWeapon::Bottle:
		case TFDetailWeapon::BrassBeast:
		case TFDetailWeapon::ConscientiousObjector:
		case TFDetailWeapon::DirectHit:
		case TFDetailWeapon::DisciplinaryAction:
		case TFDetailWeapon::EscapePlan:
		case TFDetailWeapon::EurekaEffect:
		case TFDetailWeapon::Flamethrower:
		case TFDetailWeapon::FlareGun:
		case TFDetailWeapon::ForceANature:
		case TFDetailWeapon::FryingPan:
		case TFDetailWeapon::GlovesOfRunningUrgently:
		case TFDetailWeapon::HamShank:
		case TFDetailWeapon::Homewrecker:
		case TFDetailWeapon::HuoLongHeater:
		case TFDetailWeapon::Letranger:
		case TFDetailWeapon::LooseCannon:
		case TFDetailWeapon::Minigun:
		case TFDetailWeapon::Natascha:
		case TFDetailWeapon::NecroSmasher:
		case TFDetailWeapon::Original:
		case TFDetailWeapon::Pistol:
		case TFDetailWeapon::Pomson:
		case TFDetailWeapon::Powerjack:
		case TFDetailWeapon::PrettyBoysPocketPistol:
		case TFDetailWeapon::PrinnyMachete:
		case TFDetailWeapon::RescueRanger:
		case TFDetailWeapon::ReserveShooter:
		case TFDetailWeapon::Revolver:
		case TFDetailWeapon::RocketLauncher:
		case TFDetailWeapon::ScoutBall:
		case TFDetailWeapon::Scattergun:
		case TFDetailWeapon::ScotsmansSkullcutter:
		case TFDetailWeapon::Shahanshah:
		case TFDetailWeapon::Shortstop:
		case TFDetailWeapon::Shotgun:
		case TFDetailWeapon::SkullBat:
		case TFDetailWeapon::SodaPopper:
		case TFDetailWeapon::SolemnVow:
		case TFDetailWeapon::Tomislav:
		case TFDetailWeapon::TribalmansShiv:
		case TFDetailWeapon::Ubersaw:
		case TFDetailWeapon::UnarmedCombat:
		case TFDetailWeapon::WarriorsSpirit:
		case TFDetailWeapon::Widowmaker:
		case TFDetailWeapon::Wrench:
		case TFDetailWeapon::Jag:
		case TFDetailWeapon::FreedomStaff:
		case TFDetailWeapon::PainTrain:
		case TFDetailWeapon::PanicAttack:
		case TFDetailWeapon::BackScatter:
		case TFDetailWeapon::WrapAssassin:
		case TFDetailWeapon::LibertyLauncher:
		case TFDetailWeapon::BackScratcher:
		case TFDetailWeapon::ReflectRocket:
		case TFDetailWeapon::ScorchShot:
		case TFDetailWeapon::IronBomber:
		case TFDetailWeapon::LochNLoad:
		case TFDetailWeapon::HotHand:
		case TFDetailWeapon::KillingGlovesOfBoxing:
		case TFDetailWeapon::FistsOfSteel:
		case TFDetailWeapon::Fists:
		case TFDetailWeapon::Amputator:
		case TFDetailWeapon::Blutsauger:
		case TFDetailWeapon::Bonesaw:
		case TFDetailWeapon::CrusadersCrossbow:
		case TFDetailWeapon::Overdose:
		case TFDetailWeapon::SyringeGun:
		case TFDetailWeapon::FanOWar:
		case TFDetailWeapon::Sandman:
		case TFDetailWeapon::ReflectPill:
		case TFDetailWeapon::Degreaser:
		case TFDetailWeapon::Detonator:
		case TFDetailWeapon::QuickiebombLauncher:
		case TFDetailWeapon::ScottishResistance:
		case TFDetailWeapon::GrenadeLauncher:
		case TFDetailWeapon::StickyLauncher:
		case TFDetailWeapon::FamilyBusiness:
		case TFDetailWeapon::SMG:
			return true;

		case TFDetailWeapon::Unknown:
		case TFDetailWeapon::Environment:
		case TFDetailWeapon::Club:
		//default:
			cc::dbg << "Unknown TFDetailWeapon " << weapon << " in " __FUNCTION__ << std::endl;
			return true;
	}

	throw std::invalid_argument("Out of range TFDetailWeapon");
}
#pragma warning(pop)

bool RandomCritListener::AreTeamsOpposite(TFTeam t1, TFTeam t2)
{
	return (t1 == TFTeam::Red && t2 == TFTeam::Blue) || (t1 == TFTeam::Blue && t2 == TFTeam::Red);
}

void RandomCritListener::RecordMedigunUsage(uint_fast16_t medicUID, uint_fast32_t tick)
{
	m_PlayerInfos[medicUID].m_LastKritzChargeTick = tick;
}

void RandomCritListener::CheckForMatchingEvents(const WorldState& world)
{
	for (auto hurt = m_HurtEvents.begin(); hurt != m_HurtEvents.end();)
	{
		for (auto death = m_DeathEvents.begin(); death != m_DeathEvents.end();)
		{
			if (death->m_Tick != hurt->m_Tick ||
				death->m_VictimUID != hurt->m_VictimUID || death->m_AttackerUID != hurt->m_AttackerUID ||
				death->m_WeaponID != hurt->m_WeaponID ||
				!!(death->m_DamageBits & DMG_CRITICAL) != (hurt->m_Crit || hurt->m_Minicrit))
			{
				++death;
				continue;
			}

			// We match!
			OnPlayerDeath(world, *hurt, *death);
			death = m_DeathEvents.erase(death);
			hurt = m_HurtEvents.erase(hurt);
			goto SkipHurtIncrement;
		}
		++hurt;

	SkipHurtIncrement:
		continue;
	}
}

Vector RandomCritListener::GetPlayerPosition(const Entity& player)
{
	constexpr auto localName = "DT_TFLocalPlayerExclusive.m_vecOrigin"sv;
	constexpr auto nonLocalName = "DT_TFNonLocalPlayerExclusive.m_vecOrigin"sv;

	const auto& local = player[localName];
	const auto& nonLocal = player[nonLocalName];

	Vector retVal = vec3_nan;

	if (local.GetLastChangedTick() > nonLocal.GetLastChangedTick())
		retVal = player.FindPropVec3(localName);
	else
		retVal = player.FindPropVec3(nonLocalName);

	return retVal;
}

bool RandomCritListener::IsRoundActive() const
{
	assert(m_RoundState);
	switch (*m_RoundState)
	{
		case RoundState::Running:
			return true;

		case RoundState::GameOver:
		case RoundState::TeamWin:
		case RoundState::BetweenRounds:
			return false;
	}

	assert(!"Unknown round state");
	return false;
}
