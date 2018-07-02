#pragma once

#include "TFClassType.hpp"
#include "TFWeapon.hpp"

#include <ostream>
#include <string_view>

enum class TFDetailWeapon
{
	// Multi-class
	ConscientiousObjector,
	Environment,
	FreedomStaff,
	FryingPan,
	GoldenFryingPan = FryingPan,
	HamShank,
	HalfZatoichi,
	NecroSmasher,
	PainTrain,
	PanicAttack,
	Pistol,
	PrinnyMachete,
	ReserveShooter,
	Shotgun,
	SkullBat,
	Telefrag,
	UnarmedCombat,

	// Scout
	Atomizer,
	BabyFacesBlaster,
	BackScatter,
	Bat,
	HolyMackerel = Bat,
	BostonBasher,
	ThreeRuneBlade = BostonBasher,
	FanOWar,
	ForceANature,
	PrettyBoysPocketPistol,
	Sandman,
	ScoutBall,
	Scattergun,
	Shortstop,
	SodaPopper,
	WrapAssassin,

	// Soldier
	Airstrike,
	BeggarsBazooka,
	BlackBox,
	CowMangler,
	DirectHit,
	EscapePlan,
	DisciplinaryAction,
	LibertyLauncher,
	MarketGardener,
	Original,
	RocketLauncher,
	SoldierGrenadeTaunt,

	// Pyro
	BackScratcher,
	Backburner,
	ReflectRocket,
	ReflectPill,
	ReflectHuntsman,
	ReflectHuntsmanBurning = ReflectHuntsman,
	Degreaser,
	Detonator,
	DragonsFury,
	DragonsFuryCombo = DragonsFury,
	Flamethrower,
	NostromoNapalmer = Flamethrower,
	Rainblower = Flamethrower,
	Homewrecker,
	Maul = Homewrecker,
	HotHand,
	FlareGun,
	Phlogistinator,
	Powerjack,
	ScorchShot,

	// Demo
	Bottle,
	ScottishHandshake = Bottle,
	Claidheamohmor,
	Eyelander,
	IronBomber,
	LochNLoad,
	LooseCannon,
	LooseCannonImpact = LooseCannon,
	QuickiebombLauncher,
	ScotsmansSkullcutter,
	ScottishResistance,
	GrenadeLauncher,
	StickyLauncher,
	UllapoolCaber,
	UllapoolCaberExplosion = UllapoolCaber,

	// Heavy
	BrassBeast,
	FamilyBusiness,
	GlovesOfRunningUrgently,
	HuoLongHeater,
	KillingGlovesOfBoxing,
	Minigun,
	IronCurtain = Minigun,
	Natascha,
	Fists,
	FistsOfSteel,
	Tomislav,
	WarriorsSpirit,

	// Engie
	EurekaEffect,
	FrontierJustice,
	Gunslinger,
	GunslingerTaunt = Gunslinger,
	GunslingerCombo = Gunslinger,
	Pomson,
	SentryMini,
	SentryLevel1,
	SentryLevel2,
	SentryLevel3,
	SentryRocket,
	RescueRanger,
	Widowmaker,
	Wrangler,
	Wrench,
	Jag,

	// Medic
	Amputator,
	Blutsauger,
	Bonesaw,
	CrusadersCrossbow,
	Overdose,
	SolemnVow,
	SyringeGun,
	Ubersaw,
	UbersawTaunt = Ubersaw,

	// Sniper
	AwperHand,
	BazaarBargain,
	Bushwacka,
	Machina,
	MachinaPenetration = Machina,
	ShootingStar = Machina,
	HitmansHeatmaker,
	CleanersCarbine,
	Shahanshah,
	SMG,
	SniperRifle,
	SydneySleeper,
	Huntsman,
	Classic,
	TribalmansShiv,

	// Spy
	Ambassador,
	BigEarner,
	Diamondback,
	Enforcer,
	YourEternalReward,
	WangaPrick = YourEternalReward,
	Knife,
	BlackRose = Knife,
	SharpDresser,
	Kunai,
	Letranger,
	Revolver,
	SpyCicle,

	Unknown,
	Club,

	// Multi-class duplicated items
	ShotgunSoldier = Shotgun,
	ShotgunEngie = Shotgun,
	ShotgunPyro = Shotgun,
	PistolScout = Pistol,
};

TFDetailWeapon FindTFDetailWeapon(const std::string_view& str, TFWeapon baseWeaponType);
const std::string_view& FindTFDetailWeapon(TFDetailWeapon weapon);

inline std::ostream& operator<<(std::ostream& s, TFDetailWeapon weapon)
{
	return s << FindTFDetailWeapon(weapon);
}