#pragma once

#include <cstdint>
#include <ostream>

enum class GameEventDataType : uint_fast8_t
{
	Local = 0,  // Not networked
	String,     // Zero terminated ASCII string
	Float,      // Float 32 bit
	Long,       // Signed int 32 bit
	Short,      // Signed int 16 bit
	Byte,       // Unsigned int 8 bit
	Bool,       // Unsigned int 1 bit
};

const char* EnumToString(GameEventDataType type);

inline std::ostream& operator<<(std::ostream& os, const GameEventDataType& cmd)
{
	return os << EnumToString(cmd);
}