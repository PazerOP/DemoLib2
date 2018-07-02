#pragma once

#include "net/netmessages/INetMessage.hpp"

class NetGetCvarValueMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_GETCVARVALUE; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetGetCvarValueMessage(); }

private:
	static constexpr auto COOKIE_BITS = 32;
	static constexpr auto MAX_CVAR_NAME_SIZE = 256;

	int m_Cookie;
	std::string m_CvarName;
};