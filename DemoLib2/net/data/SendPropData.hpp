#pragma once
#include "misc/Vector.hpp"
#include "misc/VectorXY.hpp"

union SendPropData
{
	int32_t m_Int;
	float m_Float;
	Vector m_Vector;
	VectorXY m_VectorXY;

	char* m_String;	// Buffer is always of size MAX_DT_STRING_LENGTH, since not many strings are used in datatables anyway

	SendPropData* m_Array;
};