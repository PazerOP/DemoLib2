#include "BitPosition.hpp"

BitPosition BitPosition::DebugSubtract(const BitPosition& other) const
{
	if (other > *this)
		throw std::domain_error("Negative BitPositions are not supported");

	BitPosition retVal;

	retVal.m_BytePos = m_BytePos - other.m_BytePos - 1 + ((m_BitPos + 8) - other.m_BitPos) / 8;
	retVal.m_BitPos = ((m_BitPos + 8) - other.m_BitPos) % 8;

	return retVal;
}