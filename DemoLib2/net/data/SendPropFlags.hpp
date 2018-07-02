#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

enum class SendPropFlags : uint_fast32_t
{
	Unsigned = (1 << 0),

	Coord = (1 << 1),

	NoScale = (1 << 2),

	RoundDown = (1 << 3),

	RoundUp = (1 << 4),

	Normal = (1 << 5),

	Exclude = (1 << 6),

	EncodeXYZE = (1 << 7),

	InsideArray = (1 << 8),

	ProxyAlwaysYes = (1 << 9),

	ChangesOften = (1 << 10),

	IsVectorElement = (1 << 11),

	Collapsible = (1 << 12),

	CoordMP = (1 << 13),

	CoordMPLowPrecision = (1 << 14),

	CoordMPIntegral = (1 << 15),

	VarInt = Normal,
};

std::string EnumToString(SendPropFlags flags);
inline std::ostream& operator<<(std::ostream& os, const SendPropFlags& flags) { return os << EnumToString(flags); }

inline constexpr SendPropFlags operator&(SendPropFlags lhs, SendPropFlags rhs)
{
	return SendPropFlags(std::underlying_type_t<SendPropFlags>(lhs) & std::underlying_type_t<SendPropFlags>(rhs));
}
inline constexpr SendPropFlags operator|(SendPropFlags lhs, SendPropFlags rhs)
{
	return SendPropFlags(std::underlying_type_t<SendPropFlags>(lhs) | std::underlying_type_t<SendPropFlags>(rhs));
}
inline constexpr bool operator!(SendPropFlags f)
{
	return !std::underlying_type_t<SendPropFlags>(f);
}