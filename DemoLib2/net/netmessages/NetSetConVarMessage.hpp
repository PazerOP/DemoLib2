#pragma once

#include "net/netmessages/INetMessage.hpp"

#include <string>
#include <utility>
#include <vector>

class NetSetConVarMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::NET_SETCONVAR; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetSetConVarMessage(); }

private:
	std::vector<std::pair<std::string, std::string>> m_Cvars;
};