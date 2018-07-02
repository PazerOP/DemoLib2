#include "ScoreboardListener.hpp"

#include "net/data/GameEvent.hpp"
#include "net/data/KnownStringTable.hpp"
#include "net/data/UserInfo.hpp"
#include "net/entities/Entity.hpp"
#include "TF2PubStats.hpp"

#include <iomanip>
#include <iostream>

#include <Windows.h>

using namespace std::string_view_literals;

std::ostream& operator<<(std::ostream& str, TFClassType c)
{
	switch (c)
	{
		default:
			assert(!"Invalid class type");
		case TFClassType::Unknown:
			str << '?';
			break;

		case TFClassType::Scout:
			str << "Scout";
			break;
		case TFClassType::Soldier:
			str << "Soldier";
			break;
		case TFClassType::Pyro:
			str << "Pyro";
			break;
		case TFClassType::DemoMan:
			str << "Demo";
			break;
		case TFClassType::Heavy:
			str << "Heavy";
			break;
		case TFClassType::Engineer:
			str << "Engie";
			break;
		case TFClassType::Medic:
			str << "Medic";
			break;
		case TFClassType::Sniper:
			str << "Sniper";
			break;
		case TFClassType::Spy:
			str << "Spy";
			break;
	}

	return str;
}

bool ScoreboardListener::FindPlayerNameByUserID(uint_fast16_t userID, std::string& name) const
{
	const auto& userinfos = GetWS().GetStringTable(KnownStringTable::Userinfo);

	name.clear();
	for (const auto& user : *userinfos)
	{
		auto clone = user.GetUserDataReader();
		if (!clone.Length())
			continue;

		assert(clone.GetLocalPosition().IsZero());

		UserInfo info;
		info.ReadElement(clone);

		if (info.m_UserID == userID)
		{
			name = info.m_Name;
			return true;
		}
	}

	std::ostringstream nameFormat;
	nameFormat << "????? (userID " << userID << ')';
	name = nameFormat.str();
	return false;
}

void ScoreboardListener::OnGameRulesDeleted(const std::shared_ptr<Entity>&)
{
	// Be safe
	m_RoundState = nullptr;
}

void ScoreboardListener::UpdatePlayerTeams(WorldState& world)
{
	if (!world.IsStringTableCreated(KnownStringTable::Userinfo))
		return;

	const auto& userinfoTable = *world.GetStringTable(KnownStringTable::Userinfo);

	for (const auto& entry : userinfoTable)
	{
		if (entry.IsEmpty())
			continue;

		BitIOReader clone = entry.GetUserDataReader();
		if (!clone.Length())
			continue;

		UserInfo info;
		info.ReadElement(clone);

		const auto index = &entry - userinfoTable.begin();

		const auto& ent = world.m_Entities[index + 1];
		if (!ent || !ent->IsInPVS())
			continue;

		for (const auto& prop : ent->GetProperties())
		{
			if (prop.GetName() == "m_iTeamNum")
				m_Stats[info.m_UserID].m_Team = (TFTeam)prop.Get<int>();
			else if (prop.GetName() == "m_iClass")
				m_Stats[info.m_UserID].m_Class = (TFClassType)prop.Get<int>();
		}
	}
}

void ScoreboardListener::UpdatePlayerDPMs(uint_fast32_t tickDelta)
{
	for (auto& playerStats : m_Stats)
	{
		if (m_RoundState && *m_RoundState == RoundState::Running)
			playerStats.second.m_RoundTicks += tickDelta;
	}
}

std::string ScoreboardListener::BuildScoreboard() const
{
	std::stringstream str;

	std::pair<uint_fast16_t, PlayerEventData>* sorted = (decltype(sorted))_alloca(sizeof(*sorted) * m_Stats.size());
	std::partial_sort_copy(m_Stats.begin(), m_Stats.end(), sorted, sorted + m_Stats.size(),
		[](const auto& a, const auto& b)
	{
		return ((a.second.m_RoundTicks ? a.second.GetDPM() : 0) > (b.second.m_RoundTicks ? b.second.GetDPM() : 0));
	});

	str <<
		"=====================================================================================================\n"
		"  Player                          Kills    Deaths    KDR    KS    Team    Class    Healing    DPM    \n"
		"=====================================================================================================\n";

	size_t printed = 0;
	std::string nameBuf;
	for (size_t i = 0; i < m_Stats.size(); i++)
	{
		const auto& playerStats = sorted[i];
		if (!FindPlayerNameByUserID(playerStats.first, nameBuf))
			continue;

		if (playerStats.second.m_Team == TFTeam::Red)
			str << cc::fg::dark::red;
		else if (playerStats.second.m_Team == TFTeam::Blue)
			str << cc::fg::blue;

		str << ' ' << std::setw(32) << std::left << nameBuf;

		str << ' ' << std::setw(8) << std::left << playerStats.second.m_Kills;
		str << ' ' << std::setw(9) << std::left << playerStats.second.m_Deaths;

		// KDR
		if (playerStats.second.m_Kills || playerStats.second.m_Deaths)
			str << ' ' << std::setw(6) << std::left << std::fixed << std::setprecision(1) << playerStats.second.m_Kills / (float)playerStats.second.m_Deaths;
		else
			str << "       ";

		// Killstreak
		if (playerStats.second.m_Killstreak)
			str << ' ' << std::setw(5) << std::left << playerStats.second.m_Killstreak;
		else
			str << "      ";

		// Team
		if (playerStats.second.m_Team == TFTeam::Red)
			str << " Red    ";
		else if (playerStats.second.m_Team == TFTeam::Blue)
			str << " Blue   ";
		else
			str << "        ";

		// Class
		str << ' ' << std::setw(8) << std::left << playerStats.second.m_Class;

		// Healing
		str << ' ' << std::setw(10) << std::left << playerStats.second.m_Healing;

		// DPM
		if (playerStats.second.m_RoundTicks)
		{
			str << ' ' << std::setw(7) << std::left << playerStats.second.GetDPM();
		}
		else
			str << "       ";

		str << cc::clear::line << cc::reset << '\n';

		printed++;
	}

	//for (size_t i = 0; i < 32 - printed; i++)
	//	str << cc::clear::line << '\n';

	return str.str();
}

void ScoreboardListener::OnPlayerHurt(const GameEvent& event)
{
	const auto attackerID = event.GetInt("attacker");
	if (!attackerID)
		return;

	auto& attacker = m_Stats[attackerID];
	if (attacker.m_Class == TFClassType::Spy)
		return;	// Spies get no damage stats until i figure out a better way to handle them

	if (attackerID == event.GetInt("userid"))
		return;	// Don't count self damage

	const auto damage = event.GetInt("damageamount");

	attacker.m_Damage += damage;
}

void ScoreboardListener::OnPlayerHealed(const GameEvent& event)
{
	const auto patient = event.GetInt("patient");
	const auto healer = event.GetInt("healer");
	const auto amount = event.GetInt("amount");

	m_Stats[healer].m_Healing += amount;
}

void ScoreboardListener::OnPlayerDeath(const GameEvent& event)
{
	const auto victimID = event.GetInt("userid");
	if (!victimID)
		return;

	auto& victim = m_Stats[victimID];
	victim.m_Deaths++;
	victim.m_Killstreak = 0;

	const auto attackerID = event.GetInt("attacker");
	if (!attackerID)
		return;

	auto& attacker = m_Stats[attackerID];

	attacker.m_Kills++;
	attacker.m_Killstreak++;
}

void ScoreboardListener::OnPlayerSpawn(const GameEvent& event)
{
	const auto spawnerID = event.GetInt("userid");

	auto& spawner = m_Stats[spawnerID];

	spawner.m_Team = (TFTeam)event.GetInt("team");
	spawner.m_Class = (TFClassType)event.GetInt("class");
}

ScoreboardListener::ScoreboardListener(const std::shared_ptr<WorldState>& world) : m_World(world)
{
	world->m_Events.AddEventListener(this);
}

ScoreboardListener::~ScoreboardListener()
{
	if (auto locked = m_World.lock())
		locked->m_Events.RemoveEventListener(this);
}

void ScoreboardListener::PrintConsoleOutput() const
{
	cc::out << /*cc::clear::screen <<*/ BuildScoreboard() << cc::endl;
}

void ScoreboardListener::EntityCreated(const std::shared_ptr<Entity>& ent)
{
	if (!ent->Is("DT_TFGameRulesProxy"sv, false))
		return;

	Entity& e = *ent;

	m_RoundState = &(e["DT_TeamplayRoundBasedRules.m_iRoundState"sv].Get<RoundState>());
	m_bInSetup = &(e["DT_TeamplayRoundBasedRules.m_bInSetup"sv].Get<int>());
}

void ScoreboardListener::GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event)
{
	const auto name = event->GetDeclaration().m_Name.c_str();

	if (!_stricmp(name, "player_hurt"))
		OnPlayerHurt(*event);
	else if (!_stricmp(name, "player_healed"))
		OnPlayerHealed(*event);
	else if (!_stricmp(name, "player_death"))
		OnPlayerDeath(*event);
	else if (!_stricmp(name, "player_spawn"))
		OnPlayerSpawn(*event);
}

void ScoreboardListener::TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta)
{
	//UpdatePlayerTeams(*world);
	UpdatePlayerDPMs(delta);
}

uint_fast16_t ScoreboardListener::PlayerEventData::GetDPM() const
{
	return uint_fast16_t(m_Damage / (m_RoundTicks / (66 * 60.0f)));
}
