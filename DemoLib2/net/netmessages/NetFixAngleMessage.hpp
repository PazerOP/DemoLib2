#pragma once

#include "net/netmessages/INetMessage.hpp"

class NetFixAngleMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_FIXANGLE; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetFixAngleMessage(); }

private:
  static constexpr uint_fast8_t ANGLE_BITS = 16;

  bool m_Relative;
  float m_Angle[3];
};