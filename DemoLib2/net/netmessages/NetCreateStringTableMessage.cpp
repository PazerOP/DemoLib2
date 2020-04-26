#include "NetCreateStringTableMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "interface/CmdArgs.hpp"
#include "misc/Exceptions.hpp"
#include "misc/Util.hpp"
#include "net/data/StringTable.hpp"
#include "net/data/StringTableUpdate.hpp"
#include "net/worldstate/WorldState.hpp"
#include <snappy.h>
#include <snappy-sinksource.h>

#include <cassert>
#include <iostream>
#include <vector>

class BitIOReaderSource final : public snappy::Source
{
public:
	BitIOReaderSource(const BitIOReader& reader) : m_Reader(reader) { }

	size_t Available() const override
	{
		auto remaining = m_Reader.Remaining();
		assert(remaining.Bits() == 0);
		return remaining.Bytes();
	}

	const char* Peek(size_t* len) override
	{
		*len = std::min<size_t>(BUFFER_SIZE - m_BufferOffset, Available());
		if (!m_Buffer)
			m_Buffer.reset(new std::byte[BUFFER_SIZE]);

		if (m_BufferInvalidated)
		{
			m_Reader.PeekArray(m_Buffer.get(), *len);
			m_BufferInvalidated = false;
		}

		return reinterpret_cast<const char*>(m_Buffer.get() + m_BufferOffset);
	}

	void Skip(size_t n) override
	{
		m_Reader.SeekBytes(n);

		m_BufferOffset += n;
		if (m_BufferOffset >= BUFFER_SIZE)
		{
			m_BufferInvalidated = true;
			m_BufferOffset = 0;
		}
	}

	void Reset()
	{
		m_Reader.Seek(BitPosition::Zero(), Seek::Start);
		//m_BufferSize = 0;
		m_BufferOffset = 0;
		m_BufferInvalidated = true;
	}

private:
	bool m_BufferInvalidated = true;
	static constexpr size_t BUFFER_SIZE = 4096;
	size_t m_BufferOffset = 0;
	std::unique_ptr<std::byte[]> m_Buffer;

	BitIOReader m_Reader;
};

class CheckedByteArraySink final : public snappy::UncheckedByteArraySink
{
public:
	CheckedByteArraySink(char* buffer, size_t capacity) :
		UncheckedByteArraySink(buffer), m_Capacity(capacity)
	{
		m_Position = 0;
	}

	void Append(const char* data, size_t n) override
	{
		assert(m_Position + n <= m_Capacity);
		UncheckedByteArraySink::Append(data, n);
		m_Position += n;
	}

#if 0
	void AppendAndTakeOwnership(char* bytes, size_t n, void(*deleter)(void*, const char*, size_t), void* deleter_arg) override
	{
		assert(m_Position + n <= m_Capacity);
		UncheckedByteArraySink::AppendAndTakeOwnership(bytes, n, deleter, deleter_arg);
		m_Position += n;
	}
#endif

private:
	const size_t m_Capacity;
	size_t m_Position;
};

void NetCreateStringTableMessage::ReadElementInternal(BitIOReader& reader)
{
	if (reader.ReadInline<char>() == ':')
	{
		m_IsFilenames = true;
	}
	else
	{
		reader.SeekBits(-8);
		m_IsFilenames = false;
	}

	m_TableName = reader.ReadString("m_TableName");

	reader.Read("m_MaxEntries", m_MaxEntries, 16);
	const auto encodeBits = StringTable::GetEncodeBits(m_MaxEntries) + 1; // 1 extra unnecessary bit on create
	reader.Read("m_Entries", m_Entries, encodeBits);

	auto bitCount = reader.ReadUVarInt();

	const bool isUserDataFixedSize = reader.ReadBit("userdatafixedsize");
	if (isUserDataFixedSize)
	{
		reader.Read("m_UserDataSize", m_UserDataSize, USER_DATA_SIZE_BITS);
		reader.Read("m_UserDataSizeBits", m_UserDataSizeBits, USER_DATA_SIZE_BITS_BITS);
	}

	m_WasDataCompressed = reader.ReadBit("m_WasDataCompressed");

	m_RawData = reader.TakeSpan(BitPosition::FromBits(bitCount));

	if (m_WasDataCompressed)
	{
		const auto decompressedSize = m_RawData.ReadInline<uint32_t>("m_DecompressedSize");
		const auto compressedNumBytes = m_RawData.ReadInline<uint32_t>("compressedNumBytes");

		// The "SNAP" header is not actually part of the snappy codec, it's added by Source
		char magic[4];
		m_RawData.ReadArray(magic, std::size(magic));

		if (magic[0] != 'S' || magic[1] != 'N' || magic[2] != 'A' || magic[3] != 'P')
			throw std::runtime_error("Unknown format for compressed stringtable");

		BitIOReaderSource source(m_RawData.TakeSpan(BitPosition::FromBytes(compressedNumBytes - 4)));

		uint32_t snappyDecompressedNumBytes;
		if (!snappy::GetUncompressedLength(&source, &snappyDecompressedNumBytes))
			throw std::runtime_error("Failed to snappy::GetUncompressedLength() on stringtable");
		if (snappyDecompressedNumBytes != decompressedSize)
			throw std::runtime_error("Uncompressed length reported by snappy does not match that reported by game");

		auto decompressedData = std::shared_ptr<std::byte[]>(new std::byte[snappyDecompressedNumBytes]);

		source.Reset();
		CheckedByteArraySink sink(reinterpret_cast<char*>(decompressedData.get()), snappyDecompressedNumBytes);
		if (!snappy::Uncompress(&source, &sink))
			throw std::runtime_error("Failed to decompress snappy data");

		m_DecompressedData = BitIOReader(decompressedData, BitPosition::FromBytes(snappyDecompressedNumBytes));
	}
	else
	{
		m_DecompressedData = m_RawData;
	}

	m_TableUpdate.emplace(encodeBits, m_MaxEntries, isUserDataFixedSize, m_UserDataSizeBits.value_or(0), m_UserDataSize.value_or(0), m_Entries);
	{
		auto clone = m_DecompressedData;
		m_TableUpdate->ReadElement(clone);
		assert(clone.Remaining().Bytes() == 0);
	}
}

BitIOWriter NetCreateStringTableMessage::Compress() const
{
	BitIOWriter compressedWriter(true);

	const auto sizeInfoPos = compressedWriter.GetPosition();
	compressedWriter.Write<uint64_t>(0); // god damn im an epic programmer (seek 2 x uint32 ahead, we'll fill this in later)

	// Magic header
	compressedWriter.Write('S');
	compressedWriter.Write('N');
	compressedWriter.Write('A');
	compressedWriter.Write('P');

	BitIOWriter decompressedWriter(true);
	m_TableUpdate->WriteElement(decompressedWriter);
	decompressedWriter.PadToByte();
	size_t decompressedSize = decompressedWriter.Length().TotalBytes();

	decompressedWriter.Seek(BitPosition::Zero(), Seek::Start);
	BitIOReaderSource source(decompressedWriter);

	struct BitIOWriterSink final : public snappy::Sink
	{
		BitIOWriterSink(BitIOWriter& writer) : m_Writer(writer) {}
		void Append(const char* bytes, size_t n) override { m_Writer.WriteChars(bytes, n); }

	private:
		BitIOWriter& m_Writer;
	};

	BitIOWriterSink sink(compressedWriter);
	size_t compressedSize = snappy::Compress(&source, &sink);

	compressedWriter.Seek(sizeInfoPos, Seek::Set);
	compressedWriter.Write(uint32_t(decompressedSize));
	compressedWriter.Write(uint32_t(compressedSize + 4)); // The SNAP "header" is included in this size

	return compressedWriter;
}

void NetCreateStringTableMessage::WriteElementInternal(BitIOWriter& writer) const
{
	if (m_IsFilenames)
		writer.Write(':');

	writer.Write(m_TableName);

	const auto encodeBits = StringTable::GetEncodeBits(m_MaxEntries) + 1; // 1 extra unnecessary bit on create
	writer.Write<uint16_t>(m_MaxEntries);
	const auto entries = m_TableUpdate->GetEntryCount();
	writer.Write(entries, encodeBits);

	// Determine bit count
	BitIOWriter compressed = Compress();
	writer.WriteVarInt(compressed.Length().TotalBits());

	// both or neither
	if (m_UserDataSize.has_value())
	{
		assert(m_UserDataSizeBits.has_value());
		writer.Write(true);
		writer.Write(m_UserDataSize.value(), USER_DATA_SIZE_BITS);
		writer.Write(m_UserDataSizeBits.value(), USER_DATA_SIZE_BITS_BITS);
	}
	else
	{
		assert(!m_UserDataSizeBits.has_value());
		writer.Write(false);
	}

	writer.Write(true); // Using compression

	compressed.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(compressed);
}

void NetCreateStringTableMessage::GetDescription(std::ostream& description) const
{
	description << "svc_CreateStringTable: table " << m_TableName <<
		", entries " << m_MaxEntries <<
		", bytes " << m_RawData.Length().TotalBytes() <<
		" userdatasize " << m_UserDataSize <<
		", userdatabits " << m_UserDataSizeBits;
}
void NetCreateStringTableMessage::ApplyWorldState(WorldState& world) const
{
	std::shared_ptr<StringTable> table;

	bool wasUpdate = false;

	if (auto existing = std::find_if(std::begin(world.m_StringTables), std::begin(world.m_StringTables) + world.m_StringTableCount,
		[&](const auto& table) { return table->GetName() == m_TableName; }); existing != std::begin(world.m_StringTables) + world.m_StringTableCount)
	{
		table = *existing;
		wasUpdate = true;
	}
	else
	{
		table = std::make_shared<StringTable>(world.GetWeak(), copy(m_TableName), m_MaxEntries, m_UserDataSize, m_UserDataSizeBits);
	}

	// Apply the table update
	m_TableUpdate->ApplyUpdate(*table);

	if (!wasUpdate)
	{
		world.m_StringTables[world.m_StringTableCount++] = table; // Add to world
		world.m_Events.PostStringTableCreate(table);
	}
	else
		world.m_Events.PostStringTableUpdate(table);
}