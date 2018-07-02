#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetTempEntitiesMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_TEMPENTITIES; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetTempEntitiesMessage(); }

private:
  uint_fast8_t m_EntryCount;
  BitIOReader m_Data;
};