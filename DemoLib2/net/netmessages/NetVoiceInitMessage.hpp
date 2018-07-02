#pragma once

#include "net/data/VoiceSetup.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetVoiceInitMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_VOICEINIT; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetVoiceInitMessage(); }

private:
  static constexpr uint_fast8_t QUALITY_HAS_SAMPLE_RATE = 255;

  VoiceSetup m_Setup;
};