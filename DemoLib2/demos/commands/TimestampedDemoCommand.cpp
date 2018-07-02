#include "TimestampedDemoCommand.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"

#include "net/worldstate/WorldState.hpp"

#include <cinttypes>

void TimestampedDemoCommand::ReadElementInternal(BitIOReader& reader)
{
	reader.Read("TimestampedDemoCommand::m_Tick", m_Tick, 32);
}
void TimestampedDemoCommand::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Tick, 32);
}

void TimestampedDemoCommand::ApplyWorldState(WorldState& world) const
{
	const auto delta = int(m_Tick) - int(world.m_DemoTick);
	world.m_DemoTick = m_Tick;
	world.m_Events.DemoTickChanged(delta);
}
