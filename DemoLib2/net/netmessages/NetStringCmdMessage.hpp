#pragma once

#include "net/netmessages/INetMessage.hpp"

class NetStringCmdMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::NET_STRINGCMD; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetStringCmdMessage(); }

private:
	std::string m_Command;
};