#pragma once

#include "net/data/SignonState.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetSignonStateMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::NET_SIGNONSTATE; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetSignonStateMessage(); }

private:
  SignonState m_State;
};