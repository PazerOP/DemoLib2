#include "NetCrosshairAngleMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"

#include <iomanip>

void NetCrosshairAngleMessage::GetDescription(std::ostream& description) const
{
	description << "svc_CrosshairAngle: (" << std::fixed << std::setprecision(1) << m_Angles[0] << ' ' << m_Angles[1] << ' ' << m_Angles[2] << ')';
}

void NetCrosshairAngleMessage::ApplyWorldState(WorldState& world) const
{
	// Nothing to do here
}

void NetCrosshairAngleMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Angles[0] = reader.ReadBitAngle(ANGLE_BITS);
	m_Angles[1] = reader.ReadBitAngle(ANGLE_BITS);
	m_Angles[2] = reader.ReadBitAngle(ANGLE_BITS);
}

void NetCrosshairAngleMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.WriteBitAngle(m_Angles[0], ANGLE_BITS);
	writer.WriteBitAngle(m_Angles[1], ANGLE_BITS);
	writer.WriteBitAngle(m_Angles[2], ANGLE_BITS);
}
