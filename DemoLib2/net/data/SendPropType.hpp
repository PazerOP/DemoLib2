#pragma once

#include <cstdint>
#include <ostream>

enum class SendPropType : uint_fast8_t
{
	Int = 0,
	Float,
	Vector,
	VectorXY,
	String,
	Array,
	Datatable,
	//Quaternion,
	//Int64,

	COUNT
};

const char* EnumToString(SendPropType type);

inline std::ostream& operator<<(std::ostream& os, const SendPropType& cmd)
{
	return os << EnumToString(cmd);
}