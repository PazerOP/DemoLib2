#pragma once

#include "BitIO/BitIOBase.hpp"
#include "interface/CmdArgs.hpp"
#include "misc/Exceptions.hpp"
#include "misc/Util.hpp"

#include <optional>

#define BITIO_READ(reader, param) \
  reader.Read(_M_xstr(param), param)

#define BITIO_READ_BITS(reader, param, bits)  \
  reader.Read(_M_xstr(param), param, bits)

template<typename T> inline constexpr bool is_simple_type_v =
!std::is_const_v<T> && !std::is_class_v<T> && !std::is_array_v<T>;

class BitIOReader : public BitIOBase
{

public:
	BitIOReader();
	BitIOReader(const std::shared_ptr<const std::byte[]>& base, const BitPosition& length);
	BitIOReader(const std::shared_ptr<const std::byte[]>& base, const BitPosition& startPos, const BitPosition& endPos);

	BitIOReader TakeSpan(const BitPosition& length);
	BitIOReader TakeSpan(const BitPosition& startPos, const BitPosition& endPos);

	BitIOReader Span(const BitPosition& startPos, const BitPosition& endPos);

	BitIOReader SpanLocal(const BitPosition& length);
	BitIOReader SpanLocal(const BitPosition& startPos, const BitPosition& endPos);

	BitIOReader Clone() const { return *this; }

	void DebugFind(uintmax_t value, uint_fast8_t bits, uintmax_t bitsToSearch = 64);
	const std::byte* GetPtr() const;

	// Base template read/peek functions
	template<class T = uint_fast32_t> T ReadInline(uint_fast8_t bits = sizeof(T) * 8);
	template<class T, typename = std::enable_if_t<is_simple_type_v<T>>> void Read(T& out, uint_fast8_t bits = sizeof(T) * 8);
	template<class T> T PeekInline(uint_fast8_t bits = sizeof(T) * 8) const;
	template<class T = uint32_t> T PeekInline(const BitPosition& offset, uint_fast8_t bits = sizeof(T) * 8) const;
	void Peek(bool& out, uint_fast8_t bits = 1) const;
	template<class T, typename = std::enable_if_t<is_simple_type_v<T>>> void Peek(T& out, uint_fast8_t bits = sizeof(T) * 8) const;
	template<class T, typename = std::enable_if_t<is_simple_type_v<T>>> void Peek(T& out, const BitPosition& offset, uint_fast8_t bits = sizeof(T) * 8) const;

	// Peek helpers
	template<class T> void Peek(std::optional<T>& out, uint_fast8_t bits = sizeof(T) * 8) const;
	template<class T> void Peek(std::optional<T>& out, const BitPosition& offset, uint_fast8_t bits = sizeof(T) * 8) const;
	template<class T> T* PeekArray(T* buffer, size_t count, uint_fast8_t bits = sizeof(T) * 8) const;
	int_fast32_t PeekVarInt() const;
	uint_fast32_t PeekUVarInt() const;

	// Read helpers
	template<class T> void Read(std::optional<T>& out, uint_fast8_t bits = sizeof(T) * 8);
	__forceinline bool ReadBit() { return !!ReadInline<uint8_t>(1); }
	bool ReadBit(const char* debugName);
	void Read(bool& out, uint_fast8_t bits = 1);
	size_t ReadString(char* out, size_t maxBytes, bool* reachedEnd = nullptr);
	std::string ReadString(size_t maxBytes = std::numeric_limits<size_t>::max());
	int_fast32_t ReadVarInt();
	uint_fast32_t ReadUVarInt();
	Vector ReadBitVec();
	float ReadBitAngle(uint_fast8_t bits);
	float ReadBitCoord();
	template<class T> T* ReadArray(T* buffer, size_t count, uint_fast8_t bits = sizeof(T) * 8);

	// Debugging enhancers
	template<class T = uint32_t> T ReadInline(const char* debugName, uint_fast8_t bits = sizeof(T) * 8);
	template<class T, typename = std::enable_if_t<is_simple_type_v<T>>> void Read(const char* debugName, T& out, uint_fast8_t bits = sizeof(T) * 8);
	void Read(const char* debugName, bool& out, uint_fast8_t bits = 1);
	template<class T, typename = std::enable_if_t<is_simple_type_v<T>>> void Read(const char* debugName, std::optional<T>& out, uint_fast8_t bits = sizeof(T) * 8);
	std::string ReadString(const char* debugName, size_t maxBytes = std::numeric_limits<size_t>::max());
};

template<class T> __forceinline T BitIOReader::PeekInline(uint_fast8_t bits) const
{
	T retVal;
	Peek(retVal, bits);
	return retVal;
}
template<class T> __forceinline T BitIOReader::PeekInline(const BitPosition& offset, uint_fast8_t bits) const
{
	T retVal;
	Peek(retVal, offset, bits);
	return retVal;
}
template<class T, typename> __forceinline void BitIOReader::Peek(T& out, uint_fast8_t bits) const
{
	return Peek(out, BitPosition::FromBytes(0), bits);
}
template<class T, typename> __forceinline void BitIOReader::Peek(T& out, const BitPosition& offset, uint_fast8_t bits) const
{
	static_assert(!std::is_const_v<T>);
	static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>);
	static_assert(!std::is_same_v<T, bool>);
	static_assert(sizeof(T) <= 8, "Not implemented");

	const auto offsetPos = GetPosition() + offset;
	if ((offsetPos + BitPosition::FromBits(bits)) > EndPosition())
		throw std::out_of_range(__FUNCSIG__ ": Attempted to peek beyond the end");

	if constexpr (sizeof(T) <= 1)
		*reinterpret_cast<uint8_t*>(&out) = ReadUInt8(&GetBase()[offsetPos.Bytes()], bits, offsetPos.Bits());
	else if constexpr (sizeof(T) <= 2)
		*reinterpret_cast<uint16_t*>(&out) = ReadUInt16(&GetBase()[offsetPos.Bytes()], bits, offsetPos.Bits());
	else if constexpr (sizeof(T) <= 4)
		*reinterpret_cast<uint32_t*>(&out) = ReadUInt32(&GetBase()[offsetPos.Bytes()], bits, offsetPos.Bits());
	else    // <= 8 bytes
		*reinterpret_cast<uint64_t*>(&out) = ReadUInt64(&GetBase()[offsetPos.Bytes()], bits, offsetPos.Bits());
}
template<class T> __forceinline void BitIOReader::Peek(std::optional<T>& out, uint_fast8_t bits) const
{
	out.emplace(Peek<T>(bits));
}
template<class T> __forceinline void BitIOReader::Peek(std::optional<T>& out, const BitPosition& offset, uint_fast8_t bits) const
{
	out.emplace(Peek<T>(bits, offset));
}

template<class T, typename> __forceinline void BitIOReader::Read(T& out, uint_fast8_t bits)
{
	static_assert(!std::is_const_v<T>);
	static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>);
	static_assert(!std::is_same_v<T, bool>);
	static_assert(sizeof(T) <= 8, "Not implemented");

	if ((GetPosition() + BitPosition::FromBits(bits)) > EndPosition())
		throw std::out_of_range(__FUNCSIG__ ": Attempted to read beyond the end");

	if constexpr (sizeof(T) <= 1)
		*reinterpret_cast<uint8_t*>(&out) = ReadUInt8(&GetBase()[GetPosition().Bytes()], bits, GetPosition().Bits());
	else if constexpr (sizeof(T) <= 2)
		*reinterpret_cast<uint16_t*>(&out) = ReadUInt16(&GetBase()[GetPosition().Bytes()], bits, GetPosition().Bits());
	else if constexpr (sizeof(T) <= 4)
		*reinterpret_cast<uint32_t*>(&out) = ReadUInt32(&GetBase()[GetPosition().Bytes()], bits, GetPosition().Bits());
	else    // <= 8 bytes
		*reinterpret_cast<uint64_t*>(&out) = ReadUInt64(&GetBase()[GetPosition().Bytes()], bits, GetPosition().Bits());

	m_Position += BitPosition::FromBits(bits);
}

template<class T> inline T* BitIOReader::PeekArray(T* buffer, size_t count, uint_fast8_t bits) const
{
	for (size_t i = 0; i < count; i++)
		Peek(buffer[i], BitPosition::FromBits(bits) * i, bits);

	return buffer;
}

template<class T> inline T* BitIOReader::ReadArray(T* buffer, size_t count, uint_fast8_t bits)
{
	PeekArray(buffer, count, bits);
	m_Position += BitPosition::FromBits(bits) * count;

	return buffer;
}

template<class T> __forceinline T BitIOReader::ReadInline(uint_fast8_t bits)
{
	T retVal;
	Read(retVal, bits);
	return retVal;
}
template<class T> __forceinline void BitIOReader::Read(std::optional<T>& out, uint_fast8_t bits)
{
	out.emplace(ReadInline<T>(bits));
}
template<class T, typename> __forceinline void BitIOReader::Read(const char* debugName, std::optional<T>& out, uint_fast8_t bits)
{
	out.emplace(ReadInline<T>(debugName, bits));
}

template<class T> inline T BitIOReader::ReadInline(const char* debugName, uint_fast8_t bits)
{
	if (GetBaseCmdArgs().m_PrintVars && GetBaseCmdArgs().m_PrintRaw)
	{
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow << cc::bold
			<< "Beginning read of " << debugName << " (" << bits << " bits)..." << cc::endl;
	}

	T retVal = ReadInline<T>(bits);

	if (GetBaseCmdArgs().m_PrintVars)
	{
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow
			<< "Read " << debugName << " (" << bits << "): " << +retVal << cc::endl;
	}

	return retVal;
}

template<class T, typename> inline void BitIOReader::Read(const char* debugName, T& out, uint_fast8_t bits)
{
	if (GetBaseCmdArgs().m_PrintVars && GetBaseCmdArgs().m_PrintRaw)
	{
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow << cc::bold
			<< "Beginning read of " << debugName << " (" << bits << " bits)..." << cc::endl;
	}

	Read(out, bits);

	if (GetBaseCmdArgs().m_PrintVars)
	{
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow
			<< "Read " << debugName << " (" << bits << "): " << +out << cc::endl;
	}
}

inline std::string BitIOReader::ReadString(const char* debugName, size_t maxChars)
{
	if (GetBaseCmdArgs().m_PrintVars && GetBaseCmdArgs().m_PrintRaw)
	{
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow << cc::bold
			<< "Beginning read of " << debugName << " (" << maxChars << " chars max)..." << cc::endl;
	}

	std::string retVal = ReadString(maxChars);

	if (GetBaseCmdArgs().m_PrintVars)
	{
		cc::out << STR_FILEBITS(*this) << cc::fg::yellow
			<< "Read " << debugName << " (" << retVal.size() + 1 << " chars): " << retVal.c_str() << cc::endl;
	}

	return retVal;
}
__forceinline void BitIOReader::Read(const char* debugName, bool& out, uint_fast8_t bits)
{
	static_assert(sizeof(uint8_t) == sizeof(bool));
	return Read<uint8_t>(debugName, *(uint8_t*)&out, bits);
}

__forceinline void BitIOReader::Peek(bool& out, uint_fast8_t bits) const
{
	static_assert(sizeof(bool) == sizeof(uint8_t));
	out = !!PeekInline<uint_fast8_t>(bits);
}
__forceinline void BitIOReader::Read(bool& out, uint_fast8_t bits)
{
	static_assert(sizeof(bool) == sizeof(uint8_t));
	out = !!ReadInline<uint_fast8_t>(bits);
}