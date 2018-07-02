#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetEntityMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_ENTITYMESSAGE; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetEntityMessage(); }

private:
  uint_fast16_t m_EntityIndex;
  uint_fast16_t m_ClassID;

  BitIOReader m_Data;
};