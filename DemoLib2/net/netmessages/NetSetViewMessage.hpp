#pragma once

#include "net/netmessages/INetMessage.hpp"

class NetSetViewMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_SETVIEW; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetSetViewMessage(); }

private:
  uint_fast16_t m_ViewEntIndex;
};