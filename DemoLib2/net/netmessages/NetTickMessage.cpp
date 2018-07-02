#include "NetTickMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/worldstate/WorldState.hpp"

#include <cmath>

void NetTickMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read("m_Tick", m_Tick, TICK_BITS);

	m_HostFrameTime = reader.ReadInline<uint32_t>("m_HostFrameTime", FLOAT_BITS) / NET_TICK_SCALEUP;
	m_HostFrameTimeStdDev = reader.ReadInline<uint32_t>("m_HostFrameTimeStdDev", FLOAT_BITS) / NET_TICK_SCALEUP;
}
void NetTickMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Tick, TICK_BITS);

	writer.Write<uint_fast32_t>(std::lround(m_HostFrameTime * NET_TICK_SCALEUP), FLOAT_BITS);
	writer.Write<uint_fast32_t>(std::lround(m_HostFrameTimeStdDev * NET_TICK_SCALEUP), FLOAT_BITS);
}

void NetTickMessage::GetDescription(std::ostream& description) const
{
	description << "net_Tick: tick " << m_Tick;
}
void NetTickMessage::ApplyWorldState(WorldState& world) const
{
	assert(m_Tick > world.m_Tick);
	const uint_fast32_t delta = m_Tick - world.m_Tick;

	world.m_Tick = m_Tick;
	world.m_ServerFrameTime = m_HostFrameTime;
	world.m_ServerFrameTimeStdDev = m_HostFrameTimeStdDev;

	world.m_Events.TickChanged(delta);
}