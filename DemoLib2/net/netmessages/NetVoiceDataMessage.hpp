#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetVoiceDataMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_VOICEDATA; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetVoiceDataMessage(); }

private:
  static constexpr uint_fast8_t VOICE_DATA_LENGTH_BITS = 16;

  uint_fast8_t m_ClientIndex;
  uint_fast8_t _unknown;
  bool m_Proximity;

  BitIOReader m_Data;
};