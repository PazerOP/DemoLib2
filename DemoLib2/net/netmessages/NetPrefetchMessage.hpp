#pragma once

#include "net/data/PrefetchType.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetPrefetchMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_PREFETCH; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetPrefetchMessage(); }

private:
  PrefetchType m_Type;
  uint_fast32_t m_SoundIndex;
};