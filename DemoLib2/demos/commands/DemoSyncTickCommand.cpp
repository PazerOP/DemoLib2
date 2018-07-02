#include "DemoSyncTickCommand.hpp"

#include "net/worldstate/WorldState.hpp"

void DemoSyncTickCommand::ApplyWorldState(WorldState& world) const
{
	TimestampedDemoCommand::ApplyWorldState(world);
	world.m_BaseTick = world.m_Tick;
}