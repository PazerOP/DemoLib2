#pragma once

#include "BitIO/BitIOReader.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetSoundMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::SVC_SOUND; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetSoundMessage(); }

private:
  static constexpr auto SOUND_COUNT_BITS = 8;
  static constexpr auto RELIABLE_SIZE_BITS = 8;
  static constexpr auto UNRELIABLE_SIZE_BITS = 16;

  bool m_Reliable;
  uint32_t m_SoundCount;
  BitIOReader m_Data;
};