#include "TFDetailWeapon.hpp"

#include <map>

#include <Windows.h>

static const std::map<std::string_view, TFDetailWeapon> s_DetailWeapons =
{
	// Scout
	{ "atomizer", TFDetailWeapon::Atomizer },
	{ "back_scatter", TFDetailWeapon::BackScatter },
	{ "ball", TFDetailWeapon::ScoutBall },
	{ "bat", TFDetailWeapon::Bat },
	{ "boston_basher", TFDetailWeapon::BostonBasher },
	{ "force_a_nature", TFDetailWeapon::ForceANature },
	{ "holymackerel", TFDetailWeapon::HolyMackerel },
	{ "pep_brawlerblaster", TFDetailWeapon::BabyFacesBlaster },
	{ "pep_pistol", TFDetailWeapon::PrettyBoysPocketPistol },	// Pretty Boy's Pocket Pistol
	{ "pistol_scout", TFDetailWeapon::PistolScout },
	{ "sandman", TFDetailWeapon::Sandman },
	{ "scattergun", TFDetailWeapon::Scattergun },
	{ "scout_sword", TFDetailWeapon::ThreeRuneBlade },
	{ "shortstop", TFDetailWeapon::Shortstop },
	{ "soda_popper", TFDetailWeapon::SodaPopper },
	{ "warfan", TFDetailWeapon::FanOWar },
	{ "wrap_assassin", TFDetailWeapon::WrapAssassin },

	// Soldier
	{ "airstrike", TFDetailWeapon::Airstrike },
	{ "dumpster_device", TFDetailWeapon::BeggarsBazooka },
	{ "blackbox", TFDetailWeapon::BlackBox },
	{ "cow_mangler", TFDetailWeapon::CowMangler },
	{ "disciplinary_action", TFDetailWeapon::DisciplinaryAction },
	{ "liberty_launcher", TFDetailWeapon::LibertyLauncher },
	{ "market_gardener", TFDetailWeapon::MarketGardener },
	{ "quake_rl", TFDetailWeapon::Original },
	{ "rocketlauncher_directhit", TFDetailWeapon::DirectHit },
	{ "shotgun_soldier", TFDetailWeapon::ShotgunSoldier },
	{ "taunt_soldier", TFDetailWeapon::SoldierGrenadeTaunt },
	{ "tf_projectile_rocket", TFDetailWeapon::RocketLauncher },
	{ "unique_pickaxe_escape", TFDetailWeapon::EscapePlan },

	// Pyro
	{ "back_scratcher", TFDetailWeapon::BackScratcher },
	{ "backburner", TFDetailWeapon::Backburner },
	{ "deflect_huntsman_flyingburn", TFDetailWeapon::ReflectHuntsmanBurning },
	{ "deflect_promode", TFDetailWeapon::ReflectPill },
	{ "deflect_rocket", TFDetailWeapon::ReflectRocket },
	{ "degreaser", TFDetailWeapon::Degreaser },
	{ "detonator", TFDetailWeapon::Detonator },
	{ "dragons_fury", TFDetailWeapon::DragonsFury },
	{ "dragons_fury_bonus", TFDetailWeapon::DragonsFuryCombo },
	{ "flamethrower", TFDetailWeapon::Flamethrower },
	{ "ai_flamethrower", TFDetailWeapon::NostromoNapalmer },	// "Alien: Isolation flamethrower"
	{ "flaregun", TFDetailWeapon::FlareGun },
	{ "hot_hand", TFDetailWeapon::HotHand },
	{ "phlogistinator", TFDetailWeapon::Phlogistinator },
	{ "powerjack", TFDetailWeapon::Powerjack },
	{ "rainblower", TFDetailWeapon::Rainblower },
	{ "scorch_shot", TFDetailWeapon::ScorchShot },
	{ "shotgun_pyro", TFDetailWeapon::ShotgunPyro },
	{ "sledgehammer", TFDetailWeapon::Homewrecker },
	{ "the_maul", TFDetailWeapon::Maul },

	// Demo
	{ "battleaxe", TFDetailWeapon::ScotsmansSkullcutter },
	{ "bottle", TFDetailWeapon::Bottle },
	{ "claidheamohmor", TFDetailWeapon::Claidheamohmor },
	{ "demokatana", TFDetailWeapon::HalfZatoichi },
	{ "iron_bomber", TFDetailWeapon::IronBomber },
	{ "loch_n_load", TFDetailWeapon::LochNLoad },
	{ "loose_cannon", TFDetailWeapon::LooseCannon },
	{ "loose_cannon_impact", TFDetailWeapon::LooseCannonImpact },
	{ "quickiebomb_launcher", TFDetailWeapon::QuickiebombLauncher },
	{ "scotland_shard", TFDetailWeapon::ScottishHandshake },
	{ "sticky_resistance", TFDetailWeapon::ScottishResistance },
	{ "sword", TFDetailWeapon::Eyelander },
	{ "tf_projectile_pipe", TFDetailWeapon::GrenadeLauncher },
	{ "tf_projectile_pipe_remote", TFDetailWeapon::StickyLauncher },
	{ "ullapool_caber", TFDetailWeapon::UllapoolCaber },
	{ "ullapool_caber_explosion", TFDetailWeapon::UllapoolCaberExplosion },

	// Heavy
	{ "brass_beast", TFDetailWeapon::BrassBeast },
	{ "family_business", TFDetailWeapon::FamilyBusiness },
	{ "fists", TFDetailWeapon::Fists },
	{ "gloves", TFDetailWeapon::KillingGlovesOfBoxing },
	{ "gloves_running_urgently", TFDetailWeapon::GlovesOfRunningUrgently },
	{ "long_heatmaker", TFDetailWeapon::HuoLongHeater },
	{ "minigun", TFDetailWeapon::Minigun },
	{ "iron_curtain", TFDetailWeapon::IronCurtain },
	{ "natascha", TFDetailWeapon::Natascha },
	{ "steel_fists", TFDetailWeapon::FistsOfSteel },
	{ "tomislav", TFDetailWeapon::Tomislav },
	{ "warrior_spirit", TFDetailWeapon::WarriorsSpirit },

	// Engie
	{ "eureka_effect", TFDetailWeapon::EurekaEffect },
	{ "frontier_justice", TFDetailWeapon::FrontierJustice },
	{ "obj_minisentry", TFDetailWeapon::SentryMini },
	{ "obj_sentrygun", TFDetailWeapon::SentryLevel1 },
	{ "obj_sentrygun2", TFDetailWeapon::SentryLevel2 },
	{ "obj_sentrygun3", TFDetailWeapon::SentryLevel3 },
	{ "pomson", TFDetailWeapon::Pomson },
	{ "rescue_ranger", TFDetailWeapon::RescueRanger },
	{ "robot_arm", TFDetailWeapon::Gunslinger },
	{ "robot_arm_blender_kill", TFDetailWeapon::GunslingerTaunt },
	{ "robot_arm_combo_kill", TFDetailWeapon::GunslingerCombo },
	{ "shotgun_primary", TFDetailWeapon::ShotgunEngie },
	{ "widowmaker", TFDetailWeapon::Widowmaker },
	{ "wrangler_kill", TFDetailWeapon::Wrangler },
	{ "wrench", TFDetailWeapon::Wrench },
	{ "wrench_jag", TFDetailWeapon::Jag },

	// Medic
	{ "amputator", TFDetailWeapon::Amputator },
	{ "blutsauger", TFDetailWeapon::Blutsauger },
	{ "bonesaw", TFDetailWeapon::Bonesaw },
	{ "crusaders_crossbow", TFDetailWeapon::CrusadersCrossbow },
	{ "proto_syringe", TFDetailWeapon::Overdose },
	{ "solemn_vow", TFDetailWeapon::SolemnVow },
	{ "syringegun_medic", TFDetailWeapon::SyringeGun },
	{ "taunt_medic", TFDetailWeapon::UbersawTaunt },
	{ "ubersaw", TFDetailWeapon::Ubersaw },

	// Sniper
	{ "awper_hand", TFDetailWeapon::AwperHand },
	{ "bazaar_bargain", TFDetailWeapon::BazaarBargain },
	{ "bushwacka", TFDetailWeapon::Bushwacka },
	{ "machina", TFDetailWeapon::Machina },
	{ "player_penetration", TFDetailWeapon::MachinaPenetration },
	{ "pro_rifle", TFDetailWeapon::HitmansHeatmaker },
	{ "pro_smg", TFDetailWeapon::CleanersCarbine },
	{ "smg", TFDetailWeapon::SMG },
	{ "shahanshah", TFDetailWeapon::Shahanshah },
	{ "shooting_star", TFDetailWeapon::ShootingStar },
	{ "sniperrifle", TFDetailWeapon::SniperRifle },
	{ "sydney_sleeper", TFDetailWeapon::SydneySleeper },
	{ "tf_projectile_arrow", TFDetailWeapon::Huntsman },
	{ "the_classic", TFDetailWeapon::Classic },
	{ "tribalkukri", TFDetailWeapon::TribalmansShiv },

	// Spy
	{ "ambassador", TFDetailWeapon::Ambassador },
	{ "big_earner", TFDetailWeapon::BigEarner },
	{ "black_rose", TFDetailWeapon::BlackRose },
	{ "diamondback", TFDetailWeapon::Diamondback },
	{ "enforcer", TFDetailWeapon::Enforcer },
	{ "eternal_reward", TFDetailWeapon::YourEternalReward },
	{ "knife", TFDetailWeapon::Knife },
	{ "kunai", TFDetailWeapon::Kunai },
	{ "letranger", TFDetailWeapon::Letranger },
	{ "revolver", TFDetailWeapon::Revolver },
	{ "sharp_dresser", TFDetailWeapon::SharpDresser },
	{ "spy_cicle", TFDetailWeapon::SpyCicle },
	{ "voodoo_pin", TFDetailWeapon::WangaPrick },

	{ "unknown", TFDetailWeapon::Unknown },
	{ "club", TFDetailWeapon::Club },

	// Multi-class
	{ "freedom_staff", TFDetailWeapon::FreedomStaff },
	{ "fryingpan", TFDetailWeapon::FryingPan },
	{ "golden_fryingpan", TFDetailWeapon::GoldenFryingPan },
	{ "ham_shank", TFDetailWeapon::HamShank },
	{ "necro_smasher", TFDetailWeapon::NecroSmasher },
	{ "nonnonviolent_protest", TFDetailWeapon::ConscientiousObjector },
	{ "paintrain", TFDetailWeapon::PainTrain },
	{ "panic_attack", TFDetailWeapon::PanicAttack },
	{ "pistol", TFDetailWeapon::Pistol },
	{ "prinny_machete", TFDetailWeapon::PrinnyMachete },
	{ "skullbat", TFDetailWeapon::SkullBat },
	{ "telefrag", TFDetailWeapon::Telefrag },
	{ "unarmed_combat", TFDetailWeapon::UnarmedCombat },
	{ "world", TFDetailWeapon::Environment },
	{ "reserve_shooter", TFDetailWeapon::ReserveShooter }
};

TFDetailWeapon FindTFDetailWeapon(const std::string_view& str, TFWeapon baseWeaponType)
{
	if (auto found = s_DetailWeapons.find(str); found != s_DetailWeapons.end())
		return found->second;

	cc::dbg << "No TFDetailWeapon entry for weapon \"" << str << '"' << std::endl;
	return TFDetailWeapon::Unknown;
}

const std::string_view& FindTFDetailWeapon(TFDetailWeapon weapon)
{
	for (const auto& entry : s_DetailWeapons)
	{
		if (entry.second == weapon)
			return entry.first;
	}

	throw std::out_of_range("Invalid TFDetailWeapon");
}
