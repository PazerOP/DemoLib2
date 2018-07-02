#pragma once

#include "net/netmessages/NetMessageType.hpp"

#include <memory>
#include <vector>

class BitIOReader;
class BitIOWriter;
class INetMessage;

class NetMessageCoder final
{
public:
  static std::vector<std::shared_ptr<INetMessage>> Decode(BitIOReader& reader);
  static void Encode(BitIOWriter& writer, const std::vector<std::shared_ptr<INetMessage>>& messages);

private:
  NetMessageCoder() = delete;
  static std::shared_ptr<INetMessage> CreateNetMessage(NetMessageType type);
};