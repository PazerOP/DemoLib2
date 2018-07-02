#include "NetFixAngleMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"

void NetFixAngleMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Relative = reader.ReadBit();

	m_Angle[0] = reader.ReadBitAngle(ANGLE_BITS);
	m_Angle[1] = reader.ReadBitAngle(ANGLE_BITS);
	m_Angle[2] = reader.ReadBitAngle(ANGLE_BITS);
}
void NetFixAngleMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Relative);

	writer.WriteBitAngle(m_Angle[0], ANGLE_BITS);
	writer.WriteBitAngle(m_Angle[1], ANGLE_BITS);
	writer.WriteBitAngle(m_Angle[2], ANGLE_BITS);
}

void NetFixAngleMessage::GetDescription(std::ostream& description) const
{
	description << "svc_FixAngle: ";

	description << (m_Relative ? "relative " : "absolute ");

	description << m_Angle[0] << ' ' << m_Angle[1] << ' ' << m_Angle[2];
}
void NetFixAngleMessage::ApplyWorldState(WorldState& world) const
{
	// We don't have a camera, ignore
	//throw NotImplementedException();
}