#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/data/UserMessageType.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetUsrMsgMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_USERMESSAGE; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetUsrMsgMessage(); }

private:
  UserMessageType m_Type;
  BitIOReader m_Data;
};