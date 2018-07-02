#pragma once

#include <array>
#include <istream>
#include <ostream>

#pragma pack(push, 1)
struct HeatmapFileHeader
{
	constexpr HeatmapFileHeader() = default;
	constexpr HeatmapFileHeader(uint32_t entryCount) : m_MagicBytes(MAGIC_BYTES), m_Version(VERSION), m_EntryCount(entryCount) {}

	static constexpr size_t SIZE = 12;

	static constexpr std::array<char, 4> MAGIC_BYTES = { 'C', 'E', 'H', 'M' };    // CastingEssentials Heat Map
	std::array<char, std::size(MAGIC_BYTES)> m_MagicBytes;

	static constexpr int VERSION = 1;
	int m_Version;

	uint32_t m_EntryCount;
};
static_assert(sizeof(HeatmapFileHeader) == HeatmapFileHeader::SIZE);

struct HeatmapFileEntry
{
	static constexpr size_t SIZE = 10;

	int16_t x;
	int16_t y;
	int16_t z;
	uint32_t count;
};
static_assert(sizeof(HeatmapFileEntry) == HeatmapFileEntry::SIZE);
#pragma pack(pop)

__forceinline std::ostream& operator<<(std::ostream& os, const HeatmapFileHeader& header)
{
	return os.write(reinterpret_cast<const char*>(&header), HeatmapFileHeader::SIZE);
}
__forceinline std::istream& operator>>(std::istream& is, HeatmapFileHeader& header)
{
	return is.read(reinterpret_cast<char*>(&header), HeatmapFileHeader::SIZE);
}

__forceinline std::ostream& operator<<(std::ostream& os, const HeatmapFileEntry& header)
{
	return os.write(reinterpret_cast<const char*>(&header), HeatmapFileEntry::SIZE);
}
__forceinline std::istream& operator>>(std::istream& is, HeatmapFileEntry& header)
{
	return is.read(reinterpret_cast<char*>(&header), HeatmapFileEntry::SIZE);
}