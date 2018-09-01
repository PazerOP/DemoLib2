#pragma once

#include "misc/Util.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <stdexcept>

class BitPosition
{
public:
	__forceinline constexpr BitPosition() noexcept : m_BytePos(0), m_BitPos(0) { }
	constexpr BitPosition(size_t bytes, uint_fast8_t bits) noexcept;
	constexpr BitPosition(const BitPosition& other) noexcept : m_BytePos(other.m_BytePos), m_BitPos(other.m_BitPos) { }

	constexpr bool operator==(const BitPosition& other) const;
	constexpr bool operator!=(const BitPosition& other) const;
	constexpr bool operator>(const BitPosition& other) const;
	constexpr bool operator>=(const BitPosition& other) const;
	constexpr bool operator<(const BitPosition& other) const;
	constexpr bool operator<=(const BitPosition& other) const;

	BitPosition operator+(const BitPosition& other) const;
	BitPosition& operator+=(const BitPosition& other);
	BitPosition operator-(const BitPosition& other) const;
	BitPosition& operator-=(const BitPosition& other);
	template<class T> BitPosition operator*(T scalar) const;
	template<class T> BitPosition& operator*=(T scalar);

	//__forceinline constexpr operator bool() const { return m_BytePos != 0 || m_BitPos != 0; }
	__forceinline constexpr bool operator!() const { return m_BytePos == 0 && m_BitPos == 0; }

	static __forceinline constexpr BitPosition Zero() { return BitPosition(0, 0); };

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	static __forceinline constexpr BitPosition FromBits(T bits) { return BitPosition(bits / 8, bits % 8); }

	static __forceinline constexpr BitPosition FromBytes(size_t bytes) { return BitPosition(bytes, 0); }

	uintmax_t TotalBits() const;
	size_t TotalBytes() const;
	__forceinline constexpr uint_fast8_t Bits() const { return m_BitPos; }
	__forceinline constexpr size_t Bytes() const { return m_BytePos; }

	__forceinline constexpr bool IsZero() const { return m_BytePos == 0 && m_BitPos == 0; }
	__forceinline constexpr bool IsByteAligned() const { return m_BitPos == 0; }

private:
	BitPosition DebugSubtract(const BitPosition& other) const;

	size_t m_BytePos;
	uint_fast8_t m_BitPos;
};

__forceinline constexpr BitPosition::BitPosition(size_t bytes, uint_fast8_t bits) noexcept :
	m_BytePos(bytes), m_BitPos(bits)
{
	assert(bits <= 7);	// bits must be in the range [0, 7]
}

inline uintmax_t BitPosition::TotalBits() const
{
	if (m_BytePos & (size_t(0xE0) << ((sizeof(m_BytePos) - 1) * 8)))
		return std::numeric_limits<uintmax_t>::max();	// We would have overflowed
	else
		return (uintmax_t)m_BytePos * 8 + m_BitPos;
}
__forceinline size_t BitPosition::TotalBytes() const
{
	return m_BytePos + (m_BitPos + 7) / 8;
}

__forceinline constexpr bool BitPosition::operator==(const BitPosition& other) const
{
	return (m_BytePos == other.m_BytePos && m_BitPos == other.m_BitPos);
}
__forceinline constexpr bool BitPosition::operator!=(const BitPosition& other) const
{
	return (m_BytePos != other.m_BytePos || m_BitPos != other.m_BitPos);
}
__forceinline constexpr bool BitPosition::operator>(const BitPosition& other) const
{
	return m_BytePos > other.m_BytePos || (m_BytePos == other.m_BytePos && m_BitPos > other.m_BitPos);
}
__forceinline constexpr bool BitPosition::operator>=(const BitPosition& other) const
{
	return m_BytePos > other.m_BytePos || (m_BytePos == other.m_BytePos && m_BitPos >= other.m_BitPos);
}
__forceinline constexpr bool BitPosition::operator<(const BitPosition& other) const
{
	return m_BytePos < other.m_BytePos || (m_BytePos == other.m_BytePos && m_BitPos < other.m_BitPos);
}
__forceinline constexpr bool BitPosition::operator<=(const BitPosition& other) const
{
	return m_BytePos < other.m_BytePos || (m_BytePos == other.m_BytePos && m_BitPos <= other.m_BitPos);
}

__forceinline BitPosition BitPosition::operator+(const BitPosition& other) const
{
	BitPosition retVal(*this);
	retVal += other;
	return retVal;
}
__forceinline BitPosition& BitPosition::operator+=(const BitPosition& other)
{
	m_BytePos = m_BytePos + other.m_BytePos + (m_BitPos + other.m_BitPos) / 8;
	m_BitPos = (m_BitPos + other.m_BitPos) % 8;

	return *this;
}
__forceinline BitPosition BitPosition::operator-(const BitPosition& other) const
{
	BitPosition retVal(*this);
	retVal -= other;
	return retVal;
}
__forceinline BitPosition& BitPosition::operator-=(const BitPosition& other)
{
	if (other > *this)
		throw std::domain_error("Negative BitPositions are not supported");

	m_BytePos = m_BytePos - other.m_BytePos - 1 + ((m_BitPos + 8) - other.m_BitPos) / 8;
	m_BitPos = ((m_BitPos + 8) - other.m_BitPos) % 8;

	return *this;
}

template<class T>
inline BitPosition BitPosition::operator*(T scalar) const
{
	BitPosition retVal(*this);
	retVal *= scalar;
	return retVal;
}
template<class T>
inline BitPosition& BitPosition::operator*=(T scalar)
{
	m_BytePos *= scalar;

	const auto scaledBits = m_BitPos * scalar;
	m_BytePos += scaledBits / 8;
	m_BitPos = scaledBits % 8;

	return *this;
}

inline std::ostream& operator<<(std::ostream& str, const BitPosition& bp)
{
	return str << bp.Bytes() << ':' << +bp.Bits();
}