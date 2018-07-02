#pragma once

#include "net/data/ServerInfo.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetServerInfoMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_SERVERINFO; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetServerInfoMessage(); }

private:
  ServerInfo m_Info;
};