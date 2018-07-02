#include "BitIO/BitIOReader.hpp"
#include "net/data/SourceConstants.hpp"

#include <algorithm>
#include <cinttypes>
#include <iostream>

BitIOReader::BitIOReader() : BitIOBase(nullptr) { }

BitIOReader::BitIOReader(const std::shared_ptr<const std::byte[]>& base, const BitPosition& length) :
	BitIOReader(base, BitPosition::Zero(), length) { }

BitIOReader::BitIOReader(const std::shared_ptr<const std::byte[]>& base, const BitPosition& startPos, const BitPosition& endPos) : BitIOBase(base)
{
	m_StartPosition = m_Position = startPos;
	m_EndPosition = endPos;
}

const std::byte* BitIOReader::GetPtr() const
{
	if (!StartPosition().IsByteAligned() || !Length().IsByteAligned())
		throw std::logic_error("BitIOReader must be byte-aligned to call GetPtr()");

	return GetBase();
}

size_t BitIOReader::ReadString(char* out, size_t maxChars, bool* reachedEnd)
{
	if (maxChars < 2)
		throw std::invalid_argument("maxChars must be at least 2 (leave room for null terminator)");

	size_t charsRead = 0;
	if (reachedEnd)
		*reachedEnd = false;

	for (size_t i = 0; i < (maxChars - 1); i++)
	{
		char current;
		Read(current);

		*(out + charsRead) = current;

		if (!current)
		{
			if (reachedEnd)
				*reachedEnd = true;

			break;
		}
		else
			charsRead++;
	}

	// Always null terminate
	assert(charsRead < maxChars);
	out[charsRead] = '\0';

	return charsRead;
}

std::string BitIOReader::ReadString(size_t maxChars)
{
	std::string retVal;

	bool reachedEnd = false;
	do
	{
		char buf[256];
		const auto charsThisLoop = std::min(std::size(buf), maxChars);
		ReadString(buf, charsThisLoop, &reachedEnd);
		retVal += (const char*)&buf[0];
		maxChars -= charsThisLoop;
	} while (!reachedEnd && maxChars > 0);

	return retVal;
}

int_fast32_t BitIOReader::ReadVarInt()
{
	return ReadUVarInt();
}

int_fast32_t BitIOReader::PeekVarInt() const
{
	return PeekUVarInt();
}

uint_fast32_t BitIOReader::PeekUVarInt() const
{
	uint_fast32_t retVal = 0;

	for (uint_fast8_t run = 0; run < 5; run++)
	{
		uint_fast8_t oneByte = PeekInline<uint_fast8_t>(BitPosition::FromBits(run * 8), 8);
		retVal |= ((oneByte & BitRange<uint_fast32_t>(0, 7)) << run * 7);

		if (!(oneByte & (1 << 7)))
			break;
	}

	return retVal;
}

uint_fast32_t BitIOReader::ReadUVarInt()
{
	auto retVal = PeekUVarInt();

	if (retVal <= BitRange<uint_fast32_t>(0, 7))
		Seek(BitPosition::FromBits(8));
	else if (retVal <= BitRange<uint_fast32_t>(0, 14))
		Seek(BitPosition::FromBits(16));
	else if (retVal <= BitRange<uint_fast32_t>(0, 21))
		Seek(BitPosition::FromBits(24));
	else if (retVal <= BitRange<uint_fast32_t>(0, 28))
		Seek(BitPosition::FromBits(32));
	else
		Seek(BitPosition::FromBits(40));

	return retVal;
}

Vector BitIOReader::ReadBitVec()
{
	uint_fast8_t firstBits;
	Read(firstBits, 3);
	bool x = firstBits & (1 << 0);
	bool y = firstBits & (1 << 1);
	bool z = firstBits & (1 << 2);

	Vector retVal(0);

	if (x)
		retVal.x = ReadBitCoord();
	if (y)
		retVal.y = ReadBitCoord();
	if (z)
		retVal.z = ReadBitCoord();

	return retVal;
}
float BitIOReader::ReadBitCoord()
{
	bool intFlag = ReadBit();
	bool fractFlag = ReadBit();

	if (intFlag || fractFlag)
	{
		uint64_t intVal = 0;
		uint64_t fractVal = 0;

		bool isNegative = ReadBit();

		// If there's an integer, read it in
		if (intFlag)
			intVal = ReadInline<uint64_t>(COORD_INTEGER_BITS) + 1;

		// If there's a fraction, read it in
		if (fractFlag)
			fractVal = ReadInline<uint64_t>(COORD_FRACTIONAL_BITS);

		// Calculate the correct floating point value
		float retVal = intVal + (fractVal * COORD_RESOLUTION);

		// Handle negative values
		if (isNegative)
			retVal = -retVal;

		return retVal;
	}

	return 0;
}
float BitIOReader::ReadBitAngle(uint_fast8_t bitCount)
{
	return ReadInline<uint32_t>(bitCount) * (360.0f / (1 << bitCount));
}
bool BitIOReader::ReadBit(const char* debugName)
{
	if (GetBaseCmdArgs().m_PrintVars && GetBaseCmdArgs().m_PrintRaw && debugName)
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow << cc::bold << "Beginning read of " << debugName << " (bool)..." << cc::endl;

	bool retVal = ReadBit();

	if (GetBaseCmdArgs().m_PrintVars && debugName)
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow << "Read " << debugName << " (bool): " << std::boolalpha << retVal << cc::endl;

	return retVal;
}

BitIOReader BitIOReader::TakeSpan(const BitPosition& length)
{
	BitIOReader retVal = SpanLocal(length);
	Seek(length);
	return retVal;
}

BitIOReader BitIOReader::TakeSpan(const BitPosition& startPos, const BitPosition& endPos)
{
	BitIOReader retVal = SpanLocal(startPos, endPos);
	Seek(endPos, Seek::Set, false);
	return retVal;
}

BitIOReader BitIOReader::Span(const BitPosition& startPos, const BitPosition& endPos)
{
	if (startPos > endPos)
		throw std::out_of_range("startPos must be in the range [0, endPos]");
	if (startPos < StartPosition())
		throw std::out_of_range("startPos must be in the range [StartPosition(), endPos]");
	if (endPos > EndPosition())
		throw std::out_of_range("endPos must be in the range [startPos, EndPosition()]");

	BitIOReader copy = *this;
	copy.m_StartPosition = copy.m_Position = startPos;
	copy.m_EndPosition = endPos;
	return copy;
}

BitIOReader BitIOReader::SpanLocal(const BitPosition& length)
{
	return SpanLocal(BitPosition::Zero(), length);
}

BitIOReader BitIOReader::SpanLocal(const BitPosition& startPos, const BitPosition& endPos)
{
	return Span(GetPosition() + startPos, GetPosition() + endPos);
}

void BitIOReader::DebugFind(uintmax_t value, uint_fast8_t bits, uintmax_t bitsToSearch)
{
	for (uintmax_t i = 0; i < bitsToSearch; i++)
	{
		auto peekValue = PeekInline<uintmax_t>(BitPosition::FromBits(i), bits);
		if (peekValue == value)
			cc::out << "DebugFind: match (" << value << ") found at offset " << i << cc::endl;
		else
			cc::out << "DebugFind: no match found at offset " << i << cc::endl;
	}
}