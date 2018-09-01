#include "BitIOWriter.hpp"

#include "BitIO/BitIOReader.hpp"
#include "net/data/SourceConstants.hpp"

#include <cmath>
#include <stdexcept>

BitIOWriter::BitIOWriter(bool autoGrow)
{
	if (autoGrow)
	{
		auto& sharedData = GetSharedData();
		sharedData.m_AutoGrowCapacity = 64;
		sharedData.m_AutoGrowHandle.reset((std::byte*)malloc(sharedData.m_AutoGrowCapacity));
		sharedData.m_Base = sharedData.m_AutoGrowHandle.get();
	}
}

void BitIOWriter::EnsureCapacity(const BitPosition& length)
{
	auto& sharedData = GetSharedData();
	const auto& minLength = GetPosition() + length - StartPosition();
	const auto& minLengthBytes = minLength.TotalBytes();

	assert(GetPosition() >= m_StartPosition);
	assert(GetPosition() <= m_EndPosition);

	if (sharedData.m_AutoGrowHandle)
	{
		if (minLengthBytes > sharedData.m_AutoGrowCapacity)
		{
			// Grow!
			auto originalPtr = sharedData.m_AutoGrowHandle.get();
			const auto newCapacity = std::max(sharedData.m_AutoGrowCapacity, minLengthBytes) * 2;
			auto newPtr = realloc((void *)originalPtr, newCapacity);
			if (newPtr)
			{
				sharedData.m_AutoGrowHandle.release();
				sharedData.m_AutoGrowHandle.reset(reinterpret_cast<std::byte *>(newPtr));
				sharedData.m_Base = sharedData.m_AutoGrowHandle.get();
				sharedData.m_AutoGrowCapacity = newCapacity;
			}
			else
				throw std::bad_alloc();
		}
	}
	else if ((GetPosition() + length) > EndPosition())
	{
		throw std::out_of_range(strfmt("Writing this much data ({0}:{1}) would overflow the buffer (remaining {2}:{3})",
			length.Bytes(), length.Bits(), Remaining().Bytes(), Remaining().Bits()));
	}
}

uint_fast8_t BitIOWriter::Write(float data)
{
	static_assert(sizeof(data) == sizeof(uint32_t));
	return Write(*reinterpret_cast<const uint32_t*>(&data), sizeof(data) * 8);
}
uint_fast8_t BitIOWriter::Write(double data)
{
	static_assert(sizeof(data) == sizeof(uint64_t));
	return Write(*reinterpret_cast<const uint64_t*>(&data), sizeof(data) * 8);
}
size_t BitIOWriter::Write(const std::string_view& str, size_t maxChars)
{
	assert(maxChars >= 2);
	size_t charsWritten = std::min(str.size(), maxChars - 1);
	WriteChars(str.data(), charsWritten);

	if (!str.size() || str[str.size() - 1] != '\0')
	{
		Write('\0');
		charsWritten += 1;
	}

	return charsWritten;
}

void BitIOWriter::Write(BitIOReader& reader)
{
	return Write(reader, reader.Remaining());
}
void BitIOWriter::Write(BitIOReader& reader, const BitPosition& writeLength)
{
	const BitPosition endPos = std::as_const(reader).GetPosition() + writeLength;

	EnsureCapacity(writeLength);

	BitPosition remaining = endPos - std::as_const(reader).GetPosition();
	while (!remaining.IsZero())
	{
		const auto bits = remaining.Bytes() ? 8 : remaining.Bits();
		Write(reader.ReadInline<uint_fast8_t>(bits), bits);
		remaining = endPos - std::as_const(reader).GetPosition();
	}
}
void BitIOWriter::WriteChars(const char* data, size_t chars)
{
	const auto& length = BitPosition::FromBytes(chars);

	EnsureCapacity(length);

	WriteBits(const_cast<std::byte*>(&GetBase()[GetPosition().Bytes()]),
		reinterpret_cast<const std::byte*>(data),
		length, GetPosition().Bits());

	m_Position += length;
	UpdateEndPosition();
}

void BitIOWriter::PadToByte()
{
	if (auto bits = Length().Bits())
		Write<uint8_t>(0, BitPosition::FromBits(8 - bits).Bits());
}

uint_fast8_t BitIOWriter::WriteBitVec(const Vector& vec)
{
	Write(vec.x != 0);
	Write(vec.y != 0);
	Write(vec.z != 0);

	uint_fast8_t retVal = 3;

	if (vec.x != 0)
		retVal += WriteBitCoord(vec.x);
	if (vec.y != 0)
		retVal += WriteBitCoord(vec.y);
	if (vec.z != 0)
		retVal += WriteBitCoord(vec.z);

	return retVal;
}

uint_fast8_t BitIOWriter::WriteBitCoord(float val)
{
	const bool isNeg = std::signbit(val);
	if (isNeg)
		val = -val;

	float intValExact;
	float fractValExact = std::modf(val, &intValExact);

	uint_fast32_t intVal = intValExact - 1;
	uint_fast32_t fractVal = fractValExact / COORD_RESOLUTION;

	Write(intVal != 0);   // integer part flag
	Write(fractVal != 0); // fractional part flag

	uint_fast8_t retVal = 2;

	if (intVal != 0 || fractVal != 0)
	{
		retVal += Write(isNeg);  // Sign

		if (intVal != 0)
			retVal += Write(intVal, COORD_INTEGER_BITS);

		if (fractVal != 0)
			retVal += Write(fractVal, COORD_FRACTIONAL_BITS);
	}

	return retVal;
}

uint_fast8_t BitIOWriter::WriteBitAngle(float angle, uint_fast8_t bitCount)
{
	return Write<uint_fast32_t>(angle / (360.0f / (1 << bitCount)), bitCount);
}

uint_fast8_t BitIOWriter::WriteVarInt(uint_fast32_t value)
{
	if constexpr (sizeof(value) != sizeof(uint32_t))
		value &= BitRange<uint_fast32_t>(0, 32);

	uint_fast8_t retVal = 0;
	uint_fast8_t byteIndex = 0;
	do
	{
		// Write the current 7 bits
		auto byte = value & BitRange<uint_fast64_t>(0, 7);
		retVal += Write(byte, 7);
		value >>= 7;

		// Because of the way VarInts work, they are somewhere between 8 and 35 bits
		if (++byteIndex < 5)
		{
			// Write 0 or 1 in the 8th bit depending on if we have any bits after us
			retVal += Write(!!value);
			if (!value)
				break;
		}
	} while (value);

	return retVal;
}