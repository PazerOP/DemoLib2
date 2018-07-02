#pragma once

#include "misc/Vector.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetBspDecalMessage final : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_BSPDECAL; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetBspDecalMessage(); }

private:
	Vector m_Position;
	uint_fast16_t m_DecalTextureIndex;
	uint_fast16_t m_EntIndex;
	uint_fast16_t m_ModelIndex;
	bool m_LowPriority;
};