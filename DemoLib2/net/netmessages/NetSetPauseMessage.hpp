#pragma once

#include "net/netmessages/INetMessage.hpp"

class NetSetPauseMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_SETPAUSE; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetSetPauseMessage(); }

private:
	bool m_Paused;
};