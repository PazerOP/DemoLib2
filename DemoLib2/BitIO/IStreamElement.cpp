#include "IStreamElement.hpp"

#include "BitIO/BitIOWriter.hpp"

//#define ENABLE_REPARSE 1

//#define SHOW_MISMATCHES 1

#include "net/netmessages/NetServerInfoMessage.hpp"

void IStreamElement::ReadElement(BitIOReader& reader)
{
#if ENABLE_REPARSE
	const auto startReadPos = reader.GetPosition();
#endif

	ReadElementInternal(reader);

#if ENABLE_REPARSE
	const auto endReadPos = reader.GetPosition();
	const auto delta = endReadPos - startReadPos;

	BitIOWriter tempWriter(true);
	WriteElementInternal(tempWriter);
	assert(tempWriter.GetLocalPosition() == delta);

	// Reparse
	tempWriter.Seek(BitPosition::Zero(), Seek::Set);
	std::unique_ptr<IStreamElement> tempElement(CreateNewInstance());
	tempElement->ReadElementInternal(tempWriter);
	assert(tempWriter.GetLocalPosition() == delta);

#if SHOW_MISMATCHES
	// Verify the bytes
	reader.Seek(startReadPos, Seek::Set);
	tempWriter.Seek(BitPosition::Zero(), Seek::Set);
	while (!tempWriter.Remaining().IsZero())
	{
		assert(tempWriter.GetLocalPosition() == (reader.GetPosition() - startReadPos));
		const uint_fast8_t bitsToRead = tempWriter.Remaining().Bytes() ? 8 : tempWriter.Remaining().Bits();
		const uint_fast8_t readOriginal = reader.ReadInline<uint_fast8_t>(bitsToRead);
		const uint_fast8_t readWritten = tempWriter.ReadInline<uint_fast8_t>(bitsToRead);

		if (readOriginal != readWritten)
		{
			const auto bitsPos = BitPosition::FromBits(bitsToRead);
			reader.Seek(bitsPos, Seek::Cur, true);
			tempWriter.Seek(bitsPos, Seek::Cur, true);

			cc::out << cc::fg::red << cc::bold << "Mismatch at " << tempWriter.GetLocalPosition() << '/' << tempWriter.EndPosition()
				<< " when reading " << bitsToRead << " bits (" << readOriginal << " vs " << readWritten << ')' << cc::endl;

			reader.Seek(bitsPos);
			tempWriter.Seek(bitsPos);

			//const auto readOriginal2 = reader.Read<uint_fast8_t>(bitsToRead);
			//const auto readWritten2 = tempWriter.Read<uint_fast8_t>(bitsToRead);
		}
	}
#endif
#endif
}

void IStreamElement::WriteElement(BitIOWriter& writer) const
{
#if ENABLE_REPARSE
	const auto startWritePos = writer.GetPosition();
#endif

	WriteElementInternal(writer);

#if ENABLE_REPARSE
	const auto endWritePos = writer.GetPosition();
	const auto delta = endWritePos - startWritePos;

	// Reparse
	writer.Seek(startWritePos, Seek::Set);
	std::unique_ptr<IStreamElement> tempElement(CreateNewInstance());
	tempElement->ReadElementInternal(writer);
	assert(writer.GetPosition() == endWritePos);
#endif
}