#pragma once

#include "net/data/FileStatus.hpp"
#include "net/netmessages/INetMessage.hpp"

class NetFileMessage : public INetMessage
{
public:
  void GetDescription(std::ostream& description) const override;
  void ApplyWorldState(WorldState& world) const override;
  NetMessageType GetType() const override { return NetMessageType::NET_FILE; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;
  IStreamElement* CreateNewInstance() const override { return new NetFileMessage(); }

private:
  uint_fast32_t m_TransferID;
  std::string m_Filename;
  FileStatus m_Status;
};