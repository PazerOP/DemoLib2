#include "BitIOCore.hpp"

#include "BitIO/BitPosition.hpp"
#include "misc/Util.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

// #define EXTRA_DEBUG 1


void BitIOCore::WriteBits(std::byte* outBuffer, const std::byte* inData, const BitPosition& dataLength, uint_fast8_t writeBitOffset)
{
	if (writeBitOffset >= 8)
		throw std::invalid_argument("writeBitOffset must be in the range [0, 7]");

	const auto wholeBytes = dataLength.Bytes();

	if (writeBitOffset == 0)
	{
		// memcpy is likely to be more optimized than anything I could write
		memcpy(outBuffer, inData, wholeBytes);

		// Write the remaining bits
		if (dataLength.Bits())
		{
			const auto& read = *reinterpret_cast<const uint8_t*>(inData + wholeBytes);
			auto& target = *reinterpret_cast<uint8_t*>(outBuffer + wholeBytes);

			const auto mask = BitRange<uint8_t>(0, dataLength.Bits());
			target &= ~mask;
			target |= (read & mask);
		}
	}
	else
	{
		BitPosition position = BitPosition::Zero();
		const auto largeCopySize = BitPosition::FromBytes((wholeBytes / 7) * 7);

		// If we're working with large amounts of data, this loop will do most of the work.
		while (position < largeCopySize)
		{
			const auto& read = *reinterpret_cast<const uint64_t*>(inData + position.Bytes());
			auto& target = *reinterpret_cast<uint64_t*>(outBuffer + position.Bytes());

			const auto& mask = BitRange<uint64_t>(0, 7 * 8);

			target &= ~(mask << writeBitOffset);
			target |= (read & mask) << writeBitOffset;

			position += BitPosition::FromBytes(7);
		}

		outBuffer += position.Bytes();

		// Now handle the remaining 7 (6 bytes + 7 bits) or fewer bytes
		const auto remaining = dataLength - position;
		assert(remaining.TotalBytes() <= 7);
		const uint_fast8_t remainingBits = remaining.TotalBits();
		auto read = ReadUInt64(inData + position.Bytes(), remainingBits);

		for (uint_fast8_t baseBit = 0; baseBit < remainingBits; baseBit += 8)
		{
			const auto& mask = BitRange<uint_fast16_t>(0, std::min<uint_fast8_t>(8, remainingBits - baseBit));

			auto& target = *reinterpret_cast<uint16_t*>(outBuffer);

			target &= ~(mask << writeBitOffset);
			target |= (read & mask) << writeBitOffset;

			read >>= 8;
			outBuffer++;
		}
	}
}