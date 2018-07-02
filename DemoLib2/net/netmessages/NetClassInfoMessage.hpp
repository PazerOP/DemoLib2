#pragma once

#include "net/netmessages/INetMessage.hpp"

#include <memory>
#include <vector>

class ServerClass;

class NetClassInfoMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::SVC_CLASSINFO; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetClassInfoMessage(); }

private:
	uint_fast16_t m_ServerClassCount;
	bool m_CreateOnClient;

	struct SimpleServerClass
	{
		uint_fast16_t m_ID;
		std::string m_Classname;
		std::string m_DatatableName;
	};

	std::vector<SimpleServerClass> m_ServerClasses;
};