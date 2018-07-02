#pragma once

#include <cstddef>
#include <cstdint>

class BitPosition;

template<class T = uint_fast32_t> __forceinline constexpr T BitRange(uint_fast8_t minBit, uint_fast8_t maxBit)
{
	return (std::numeric_limits<T>::max() << minBit) & (std::numeric_limits<T>::max() >> ((sizeof(T) * 8) - maxBit));
}

class BitIOCore
{
protected:
	BitIOCore() = default;

	static inline constexpr uint8_t ReadUInt8(const std::byte* buffer, uint_fast8_t bits = 8, uint_fast8_t readBitOffset = 0)
	{
		const auto bitRange = s_8BitRanges[bits];
		switch ((readBitOffset + bits + 7) / 8)
		{
			case 1: return (InternalReadUInt8(buffer) >> readBitOffset) & bitRange;

			default:
			case 2: return (InternalReadUInt16(buffer) >> readBitOffset) & bitRange;
		}
	}
	static inline constexpr uint16_t ReadUInt16(const std::byte* buffer, uint_fast8_t bits = 16, uint_fast8_t readBitOffset = 0)
	{
		const auto bitRange = s_16BitRanges[bits];
		switch ((readBitOffset + bits + 7) / 8)
		{
			case 1: return (InternalReadUInt8(buffer) >> readBitOffset) & bitRange;
			case 2: return (InternalReadUInt16(buffer) >> readBitOffset) & bitRange;

			default:
			case 3: return (InternalReadUInt24(buffer) >> readBitOffset) & bitRange;
		}
	}
	static inline constexpr uint32_t ReadUInt32(const std::byte* buffer, uint_fast8_t bits = 32, uint_fast8_t readBitOffset = 0)
	{
		const auto bitRange = s_32BitRanges[bits];
		switch ((readBitOffset + bits + 7) / 8)
		{
			case 1: return (InternalReadUInt8(buffer) >> readBitOffset) & bitRange;
			case 2: return (InternalReadUInt16(buffer) >> readBitOffset) & bitRange;
			case 3: return (InternalReadUInt24(buffer) >> readBitOffset) & bitRange;

			default:
			case 4: return (InternalReadUInt32(buffer) >> readBitOffset) & bitRange;

			case 5: return (InternalReadUInt40(buffer) >> readBitOffset) & bitRange;
		}
	}
	static inline constexpr uint64_t ReadUInt64(const std::byte* buffer, uint_fast8_t bits = 64, uint_fast8_t readBitOffset = 0)
	{
		const auto bitRange = s_64BitRanges[bits];
		switch ((readBitOffset + bits + 7) / 8)
		{
			case 1: return (InternalReadUInt8(buffer) >> readBitOffset) & bitRange;
			case 2: return (InternalReadUInt16(buffer) >> readBitOffset) & bitRange;
			case 3: return (InternalReadUInt24(buffer) >> readBitOffset) & bitRange;
			case 4: return (InternalReadUInt32(buffer) >> readBitOffset) & bitRange;
			case 5: return (InternalReadUInt40(buffer) >> readBitOffset) & bitRange;
			case 6: return (InternalReadUInt48(buffer) >> readBitOffset) & bitRange;
			case 7: return (InternalReadUInt56(buffer) >> readBitOffset) & bitRange;

			default:
			case 8: return (InternalReadUInt64(buffer) >> readBitOffset) & bitRange;

			case 9: return InternalReadUInt64(buffer, readBitOffset) & bitRange;
		}
	}

	static void WriteBits(std::byte* outBuffer, const std::byte* inData, const BitPosition& dataLength, uint_fast8_t writeBitOffset = 0);

private:
	static __forceinline constexpr uint64_t InternalReadUInt64(const std::byte* buffer, uint_fast8_t readBitOffset)
	{
		if (readBitOffset != 0)
		{
			const auto part1 = (InternalReadUInt64(buffer) >> readBitOffset);
			const auto part2 = uint_fast64_t(InternalReadUInt8(buffer + 8)) << (64 - readBitOffset);
			return part1 | part2;
		}
		else
			return InternalReadUInt64(buffer);
	}

	static __forceinline constexpr uint8_t InternalReadUInt8(const std::byte* buffer)
	{
		return *reinterpret_cast<const uint8_t*>(buffer);
	}
	static __forceinline constexpr uint16_t InternalReadUInt16(const std::byte* buffer)
	{
		return *reinterpret_cast<const uint16_t*>(buffer);
	}
	static __forceinline constexpr uint32_t InternalReadUInt24(const std::byte* buffer)
	{
		return uint32_t(InternalReadUInt16(buffer)) | uint32_t(InternalReadUInt8(buffer + 2)) << 16;
	}
	static __forceinline constexpr uint32_t InternalReadUInt32(const std::byte* buffer)
	{
		return *reinterpret_cast<const uint32_t*>(buffer);
	}
	static __forceinline constexpr uint64_t InternalReadUInt40(const std::byte* buffer)
	{
		return uint64_t(InternalReadUInt32(buffer)) | uint64_t(InternalReadUInt8(buffer + 4)) << 32;
	}
	static __forceinline constexpr uint64_t InternalReadUInt48(const std::byte* buffer)
	{
		return uint64_t(InternalReadUInt32(buffer)) | uint64_t(InternalReadUInt16(buffer + 4)) << 32;
	}
	static __forceinline constexpr uint64_t InternalReadUInt56(const std::byte* buffer)
	{
		return uint64_t(InternalReadUInt32(buffer)) | uint64_t(InternalReadUInt24(buffer + 4)) << 32;
	}
	static __forceinline constexpr uint64_t InternalReadUInt64(const std::byte* buffer)
	{
		return *reinterpret_cast<const uint64_t*>(buffer);
	}

	static void WriteUInt8(std::byte* buffer, uint_fast8_t value, uint_fast8_t bits, uint_fast8_t writeBitOffset);
	static constexpr uint_fast8_t s_8BitRanges[9] =
	{
		BitRange<uint_fast8_t>(0, 0),
		BitRange<uint_fast8_t>(0, 1),
		BitRange<uint_fast8_t>(0, 2),
		BitRange<uint_fast8_t>(0, 3),
		BitRange<uint_fast8_t>(0, 4),
		BitRange<uint_fast8_t>(0, 5),
		BitRange<uint_fast8_t>(0, 6),
		BitRange<uint_fast8_t>(0, 7),
		BitRange<uint_fast8_t>(0, 8),
	};

	static constexpr uint_fast16_t s_16BitRanges[17] =
	{
		BitRange<uint_fast16_t>(0, 0),
		BitRange<uint_fast16_t>(0, 1),
		BitRange<uint_fast16_t>(0, 2),
		BitRange<uint_fast16_t>(0, 3),
		BitRange<uint_fast16_t>(0, 4),
		BitRange<uint_fast16_t>(0, 5),
		BitRange<uint_fast16_t>(0, 6),
		BitRange<uint_fast16_t>(0, 7),
		BitRange<uint_fast16_t>(0, 8),
		BitRange<uint_fast16_t>(0, 9),
		BitRange<uint_fast16_t>(0, 10),
		BitRange<uint_fast16_t>(0, 11),
		BitRange<uint_fast16_t>(0, 12),
		BitRange<uint_fast16_t>(0, 13),
		BitRange<uint_fast16_t>(0, 14),
		BitRange<uint_fast16_t>(0, 15),
		BitRange<uint_fast16_t>(0, 16),
	};

	static constexpr uint_fast32_t s_32BitRanges[33] =
	{
		BitRange<uint_fast32_t>(0, 0),
		BitRange<uint_fast32_t>(0, 1),
		BitRange<uint_fast32_t>(0, 2),
		BitRange<uint_fast32_t>(0, 3),
		BitRange<uint_fast32_t>(0, 4),
		BitRange<uint_fast32_t>(0, 5),
		BitRange<uint_fast32_t>(0, 6),
		BitRange<uint_fast32_t>(0, 7),
		BitRange<uint_fast32_t>(0, 8),
		BitRange<uint_fast32_t>(0, 9),
		BitRange<uint_fast32_t>(0, 10),
		BitRange<uint_fast32_t>(0, 11),
		BitRange<uint_fast32_t>(0, 12),
		BitRange<uint_fast32_t>(0, 13),
		BitRange<uint_fast32_t>(0, 14),
		BitRange<uint_fast32_t>(0, 15),
		BitRange<uint_fast32_t>(0, 16),
		BitRange<uint_fast32_t>(0, 17),
		BitRange<uint_fast32_t>(0, 18),
		BitRange<uint_fast32_t>(0, 19),
		BitRange<uint_fast32_t>(0, 20),
		BitRange<uint_fast32_t>(0, 21),
		BitRange<uint_fast32_t>(0, 22),
		BitRange<uint_fast32_t>(0, 23),
		BitRange<uint_fast32_t>(0, 24),
		BitRange<uint_fast32_t>(0, 25),
		BitRange<uint_fast32_t>(0, 26),
		BitRange<uint_fast32_t>(0, 27),
		BitRange<uint_fast32_t>(0, 28),
		BitRange<uint_fast32_t>(0, 29),
		BitRange<uint_fast32_t>(0, 30),
		BitRange<uint_fast32_t>(0, 31),
		BitRange<uint_fast32_t>(0, 32),
	};

	static constexpr uint_fast64_t s_64BitRanges[65] =
	{
		BitRange<uint_fast64_t>(0, 0),
		BitRange<uint_fast64_t>(0, 1),
		BitRange<uint_fast64_t>(0, 2),
		BitRange<uint_fast64_t>(0, 3),
		BitRange<uint_fast64_t>(0, 4),
		BitRange<uint_fast64_t>(0, 5),
		BitRange<uint_fast64_t>(0, 6),
		BitRange<uint_fast64_t>(0, 7),
		BitRange<uint_fast64_t>(0, 8),
		BitRange<uint_fast64_t>(0, 9),
		BitRange<uint_fast64_t>(0, 10),
		BitRange<uint_fast64_t>(0, 11),
		BitRange<uint_fast64_t>(0, 12),
		BitRange<uint_fast64_t>(0, 13),
		BitRange<uint_fast64_t>(0, 14),
		BitRange<uint_fast64_t>(0, 15),
		BitRange<uint_fast64_t>(0, 16),
		BitRange<uint_fast64_t>(0, 17),
		BitRange<uint_fast64_t>(0, 18),
		BitRange<uint_fast64_t>(0, 19),
		BitRange<uint_fast64_t>(0, 20),
		BitRange<uint_fast64_t>(0, 21),
		BitRange<uint_fast64_t>(0, 22),
		BitRange<uint_fast64_t>(0, 23),
		BitRange<uint_fast64_t>(0, 24),
		BitRange<uint_fast64_t>(0, 25),
		BitRange<uint_fast64_t>(0, 26),
		BitRange<uint_fast64_t>(0, 27),
		BitRange<uint_fast64_t>(0, 28),
		BitRange<uint_fast64_t>(0, 29),
		BitRange<uint_fast64_t>(0, 30),
		BitRange<uint_fast64_t>(0, 31),
		BitRange<uint_fast64_t>(0, 32),
		BitRange<uint_fast64_t>(0, 33),
		BitRange<uint_fast64_t>(0, 34),
		BitRange<uint_fast64_t>(0, 35),
		BitRange<uint_fast64_t>(0, 36),
		BitRange<uint_fast64_t>(0, 37),
		BitRange<uint_fast64_t>(0, 38),
		BitRange<uint_fast64_t>(0, 39),
		BitRange<uint_fast64_t>(0, 40),
		BitRange<uint_fast64_t>(0, 41),
		BitRange<uint_fast64_t>(0, 42),
		BitRange<uint_fast64_t>(0, 43),
		BitRange<uint_fast64_t>(0, 44),
		BitRange<uint_fast64_t>(0, 45),
		BitRange<uint_fast64_t>(0, 46),
		BitRange<uint_fast64_t>(0, 47),
		BitRange<uint_fast64_t>(0, 48),
		BitRange<uint_fast64_t>(0, 49),
		BitRange<uint_fast64_t>(0, 50),
		BitRange<uint_fast64_t>(0, 51),
		BitRange<uint_fast64_t>(0, 52),
		BitRange<uint_fast64_t>(0, 53),
		BitRange<uint_fast64_t>(0, 54),
		BitRange<uint_fast64_t>(0, 55),
		BitRange<uint_fast64_t>(0, 56),
		BitRange<uint_fast64_t>(0, 57),
		BitRange<uint_fast64_t>(0, 58),
		BitRange<uint_fast64_t>(0, 59),
		BitRange<uint_fast64_t>(0, 60),
		BitRange<uint_fast64_t>(0, 61),
		BitRange<uint_fast64_t>(0, 62),
		BitRange<uint_fast64_t>(0, 63),
		BitRange<uint_fast64_t>(0, 64),
	};
};