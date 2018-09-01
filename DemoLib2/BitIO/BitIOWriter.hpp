#pragma once

#include "BitIO/BitIOReader.hpp"

#include <algorithm>
#include <cassert>
#include <forward_list>
#include <memory>
#include <type_traits>

class BitIOReader;

class BitIOWriter final : public BitIOReader
{
public:
	BitIOWriter() = default;
	explicit BitIOWriter(bool autoGrow);
	BitIOWriter(const std::shared_ptr<std::byte[]>& backingStorage, const BitPosition& length);
	BitIOWriter(const std::shared_ptr<std::byte[]>& backingStorage, const BitPosition& minPos, const BitPosition& maxPos);

	// Base template write functions
	template<class T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>>>
	uint_fast8_t Write(const T& data, uint_fast8_t bits = sizeof(T) * 8);

	// Helpers
	uint_fast8_t Write(bool data) { return Write<uint8_t>(data, 1); }
	uint_fast8_t Write(float data);
	uint_fast8_t Write(double data);
	void Write(BitIOReader& reader);
	void Write(BitIOReader& reader, const BitPosition& writeLength);
	size_t Write(const std::string_view& str, size_t maxChars = std::string_view::npos);
	uint_fast8_t WriteVarInt(uint_fast32_t value);
	uint_fast8_t WriteBitVec(const Vector& vector);
	uint_fast8_t WriteBitAngle(float angle, uint_fast8_t bits);
	uint_fast8_t WriteBitCoord(float data);
	void WriteChars(const char* data, size_t chars);
		
	void PadToByte(); // Adds zero bits until Length().Bits() == 0

private:
	void UpdateEndPosition() { m_EndPosition = std::max(m_EndPosition, m_Position); }

	void EnsureCapacity(const BitPosition& length);
};

template<class T, typename> inline uint_fast8_t BitIOWriter::Write(const T& data, uint_fast8_t bits)
{
	static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
	static_assert(!std::is_same_v<T, bool>);

	assert(bits > 0);
	if (bits > sizeof(T) * 8)
		throw std::invalid_argument("bits must be in the range [0, sizeof(T) * 8]");

	EnsureCapacity(GetPosition() + BitPosition::FromBits(bits));

	// Write data
	WriteBits(const_cast<std::byte*>(&GetBase()[GetPosition().Bytes()]),
		reinterpret_cast<const std::byte*>(&data), BitPosition::FromBits(bits),
		GetPosition().Bits());

	// Seek forward
	m_Position += BitPosition::FromBits(bits);
	UpdateEndPosition();
	return bits;
}