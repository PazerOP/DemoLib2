#pragma once

enum DamageBits
{
	DMG_GENERIC = 0,         // generic damage -- do not use if you want players to flinch and bleed!

	/* crushed by falling or moving object.
	   NOTE: It's assumed crush damage is occurring as a result of physics collision, so no extra physics force is generated by crush damage.
	   DON'T use DMG_CRUSH when damaging entities unless it's the result of a physics collision. You probably want DMG_CLUB instead. */
	DMG_CRUSH = (1 << 0),
	DMG_BULLET = (1 << 1),	 // shot
	DMG_SLASH = (1 << 2),	 // cut, clawed, stabbed
	DMG_BURN = (1 << 3),	 // heat burned
	DMG_VEHICLE = (1 << 4),	// hit by a vehicle
	DMG_FALL = (1 << 5),	// fell too far
	DMG_BLAST = (1 << 6),	// explosive blast damage
	DMG_CLUB = (1 << 7),	// crowbar, punch, headbutt
	DMG_SHOCK = (1 << 8),	// electric shock
	DMG_SONIC = (1 << 9),	// sound pulse shockwave
	DMG_RADIUS_MAX = (1 << 10),
	DMG_PREVENT_PHYSICS_FORCE = (1 << 11),	// Prevent a physics force
	DMG_NEVERGIB = (1 << 12),	// with this bit OR'd in, no damage type will be able to gib victims upon death
	DMG_ALWAYSGIB = (1 << 13),	// with this bit OR'd in, any damage type can be made to gib victims upon death.
	DMG_DROWN = (1 << 14),	// Drowning


	DMG_PARALYZE = (1 << 15),	// slows affected creature down
	DMG_NERVEGAS = (1 << 16),	// nerve toxins, very bad
	DMG_NOCLOSEDISTANCEMOD = (1 << 17),
	DMG_HALF_FALLOFF = (1 << 18),
	DMG_DROWNRECOVER = (1 << 19),	// drowning recovery
	DMG_CRITICAL = (1 << 20),
	DMG_USEDISTANCEMOD = (1 << 21),

	/* with this bit OR'd in, no ragdoll will be created, and the target will be quietly removed.
	   use this to kill an entity that you've already got a server-side ragdoll for */
	DMG_REMOVENORAGDOLL = (1 << 22),

	DMG_PHYSGUN = (1 << 23),		// Hit by manipulator. Usually doesn't do any damage.
	DMG_PLASMA = (1 << 24),		// Shot by Cremator
	DMG_USE_HITLOCATIONS = (1 << 25),

	DMG_DISSOLVE = (1 << 26),		// Dissolving!
	DMG_BLAST_SURFACE = (1 << 27),		// A blast on the surface of water that cannot harm things underwater
	DMG_DIRECT = (1 << 28),
	DMG_BUCKSHOT = (1 << 29),		// not quite a bullet. Little, rounder, different.


	DMG_ENERGYBEAM = DMG_RADIUS_MAX,     // laser or other high energy beam
	DMG_POISON = DMG_NOCLOSEDISTANCEMOD, // blood poisoning - heals over time like drowning damage
	DMG_RADIATION = DMG_HALF_FALLOFF,    // radiation exposure
	DMG_ACID = DMG_CRITICAL,             // toxic chemicals or acid burns
	DMG_SLOWBURN = DMG_USEDISTANCEMOD,   // in an oven
	DMG_AIRBOAT = DMG_USE_HITLOCATIONS,  // Hit by the airboat's gun
};