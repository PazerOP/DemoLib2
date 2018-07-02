#include "NetBspDecalMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"

void NetBspDecalMessage::ReadElementInternal(BitIOReader& reader)
{
	m_Position = reader.ReadBitVec();
	reader.Read("m_DecalTextureIndex", m_DecalTextureIndex, MAX_DECAL_INDEX_BITS);

	if (reader.ReadBit())
	{
		reader.Read("m_EntIndex", m_EntIndex, MAX_EDICT_BITS);
		reader.Read("m_ModelIndex", m_ModelIndex, SP_MODEL_INDEX_BITS);
	}
	else
	{
		m_EntIndex = 0;
		m_ModelIndex = 0;
	}

	reader.Read("m_LowPriority", m_LowPriority);
}
void NetBspDecalMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.WriteBitVec(m_Position);
	writer.Write(m_DecalTextureIndex, MAX_DECAL_INDEX_BITS);

	assert(!m_EntIndex == !m_ModelIndex);
	writer.Write(!!m_EntIndex);
	if (m_EntIndex)
	{
		writer.Write(m_EntIndex, MAX_EDICT_BITS);
		writer.Write(m_ModelIndex, SP_MODEL_INDEX_BITS);
	}

	writer.Write(m_LowPriority);
}

void NetBspDecalMessage::GetDescription(std::ostream& description) const
{
	//description << "svc_BspDecal: " << m_Position << ' ' << m_DecalTextureIndex << ' ' << m_EntIndex;
	description << "svc_BspDecal: tex " << m_DecalTextureIndex
		<< ", ent " << m_EntIndex
		<< ", mod " << m_ModelIndex
		<< " lowpriority " << m_LowPriority;
}
void NetBspDecalMessage::ApplyWorldState(WorldState& world) const
{
	// Ignore bsp decals for now
	//throw NotImplementedException();
}