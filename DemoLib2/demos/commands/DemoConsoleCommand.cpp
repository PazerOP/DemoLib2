#include "DemoConsoleCommand.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

#include <iostream>

void DemoConsoleCommand::ReadElementInternal(BitIOReader& _fullReader)
{
	TimestampedDemoCommand::ReadElementInternal(_fullReader);

	BitIOReader reader = _fullReader.TakeSpan(BitPosition::FromBytes(_fullReader.ReadInline<uint32_t>()));

	m_Command = reader.ReadString();

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << cc::fg::green << GetType() << ": " << m_Command << cc::endl;
}

void DemoConsoleCommand::WriteElementInternal(BitIOWriter& writer) const
{
	TimestampedDemoCommand::WriteElementInternal(writer);

	const auto size = m_Command.size() + 1;
	assert(size <= std::numeric_limits<uint32_t>::max());
	writer.Write<uint32_t>((uint32_t)size);
	const auto charsWritten = writer.Write(m_Command);

	assert(size == charsWritten);
}

void DemoConsoleCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);
}