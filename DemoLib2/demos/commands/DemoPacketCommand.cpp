#include "demos/commands/DemoPacketCommand.hpp"

#include "BitIO/BitIOWriter.hpp"
#include "net/netmessages/INetMessage.hpp"
#include "net/worldstate/WorldState.hpp"
#include "net/NetMessageCoder.hpp"

#include <cassert>
#include <cinttypes>
#include <iostream>

void DemoPacketCommand::ReadElementInternal(BitIOReader& reader)
{
	TimestampedDemoCommand::ReadElementInternal(reader);

	m_Viewpoint.ReadElement(reader);

	reader.Read(m_SequenceIn);
	reader.Read(m_SequenceOut);

	auto length = reader.ReadInline<uint32_t>();
	assert(reader.Length().IsByteAligned());

	auto data = reader.TakeSpan(BitPosition::FromBytes(length));

	m_Messages = NetMessageCoder::Decode(data);

	if (GetBaseCmdArgs().m_PrintDemo)
	{
		cc::out << STR_FILEBITS(reader) << cc::fg::green << cc::bold << GetType() << ": " <<
			"\n\tm_SequenceIn: " << m_SequenceIn <<
			"\n\tm_SequenceOut: " << m_SequenceOut <<
			"\n\tlength: " << length << cc::endl;
	}
}
void DemoPacketCommand::WriteElementInternal(BitIOWriter& writer) const
{
	TimestampedDemoCommand::WriteElementInternal(writer);

	m_Viewpoint.WriteElement(writer);

	writer.Write(m_SequenceIn);
	writer.Write(m_SequenceOut);

	BitIOWriter tempWriter(true);
	NetMessageCoder::Encode(tempWriter, m_Messages);

	assert(tempWriter.Length().TotalBytes() <= std::numeric_limits<uint32_t>::max());
	writer.Write<uint32_t>((uint32_t)tempWriter.Length().TotalBytes());
	tempWriter.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(tempWriter);

#if 1
	// Pad to nearest byte
	if (tempWriter.GetPosition().Bits())
	{
		const uint_fast8_t paddingBits = 8 - tempWriter.GetPosition().Bits();
		cc::out << cc::fg::magenta << "Padding " << GetType() << " command with " << +paddingBits << "bits of zeros" << cc::endl;
		tempWriter.Write<uint_fast8_t>(0, paddingBits);
	}
#else
	if (tempWriter.GetPosition() != m_Data.Length())
	{
		assert((tempWriter.GetPosition() - m_Messages.Length()) < BitPosition::FromByte(1));

		auto clone = m_Data;
		clone.Seek(tempWriter.GetPosition(), Seek::Start);
		tempWriter.Write(clone);
	}

	assert(tempWriter.GetPosition().Bits() == 0);
	assert(tempWriter.GetPosition().Bytes() == tempWriter.GetPosition().TotalBytes());

	writer.Write<uint32_t>(tempWriter.Length().TotalBytes());
	tempWriter.Seek(BitPosition::Zero(), Seek::Start);
	writer.Write(tempWriter);
#endif
}

void DemoPacketCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);

	world.m_DemoViewPos = m_Viewpoint.GetViewPos();

	for (const auto& msg : m_Messages)
	{
		if (!msg)
			continue;

		if (!world.m_Events.PreNetMessage(msg))
			continue;

		msg->ApplyWorldState(world);

		world.m_Events.PostNetMessage(msg);
	}
}