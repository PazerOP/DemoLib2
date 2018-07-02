#include "DemoUserCommand.hpp"

#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"

void DemoUserCommand::ReadElementInternal(BitIOReader& reader)
{
	TimestampedDemoCommand::ReadElementInternal(reader);

	reader.Read(m_OutgoingSequence);

	auto dataLength = reader.ReadInline<uint32_t>();
	m_Data = reader.TakeSpan(BitPosition::FromBytes(dataLength));

	if (GetBaseCmdArgs().m_PrintDemo)
	{
		cc::out << STR_FILEBITS(reader) << cc::fg::green << cc::bold
			<< GetType() << ":\n"
			<< "\tm_OutgoingSequence: " << m_OutgoingSequence << '\n'
			<< "\tlength: " << dataLength << cc::endl;
	}
}

void DemoUserCommand::WriteElementInternal(BitIOWriter& writer) const
{
	TimestampedDemoCommand::WriteElementInternal(writer);

	writer.Write(m_OutgoingSequence);

	assert(m_Data.Length().IsByteAligned());
	assert(m_Data.Length().Bytes() < std::numeric_limits<uint32_t>::max());
	writer.Write<uint32_t>((uint32_t)m_Data.Length().Bytes());

	auto clone = m_Data;
	clone.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(clone);
}

void DemoUserCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);
	//throw NotImplementedException(__FUNCTION__);
}