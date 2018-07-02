#include "NetMessageCoder.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/netmessages/INetMessage.hpp"
#include "net/netmessages/NetBspDecalMessage.hpp"
#include "net/netmessages/NetClassInfoMessage.hpp"
#include "net/netmessages/NetCreateStringTableMessage.hpp"
#include "net/netmessages/NetCrosshairAngleMessage.hpp"
#include "net/netmessages/NetEntityMessage.hpp"
#include "net/netmessages/NetFileMessage.hpp"
#include "net/netmessages/NetFixAngleMessage.hpp"
#include "net/netmessages/NetGameEventListMessage.hpp"
#include "net/netmessages/NetGameEventMessage.hpp"
#include "net/netmessages/NetGetCvarValueMessage.hpp"
#include "net/netmessages/NetPacketEntitiesMessage.hpp"
#include "net/netmessages/NetPrefetchMessage.hpp"
#include "net/netmessages/NetPrintMessage.hpp"
#include "net/netmessages/NetServerInfoMessage.hpp"
#include "net/netmessages/NetSetConVarMessage.hpp"
#include "net/netmessages/NetSetPauseMessage.hpp"
#include "net/netmessages/NetSetViewMessage.hpp"
#include "net/netmessages/NetSignonStateMessage.hpp"
#include "net/netmessages/NetSoundMessage.hpp"
#include "net/netmessages/NetStringCmdMessage.hpp"
#include "net/netmessages/NetTempEntitiesMessage.hpp"
#include "net/netmessages/NetTickMessage.hpp"
#include "net/netmessages/NetUpdateStringTableMessage.hpp"
#include "net/netmessages/NetUsrMsgMessage.hpp"
#include "net/netmessages/NetVoiceDataMessage.hpp"
#include "net/netmessages/NetVoiceInitMessage.hpp"
#include "net/worldstate/WorldState.hpp"

#include <iostream>

std::vector<std::shared_ptr<INetMessage>> NetMessageCoder::Decode(BitIOReader& reader)
{
	if (GetBaseCmdArgs().m_PrintNet)
		cc::out << cc::fg::cyan << cc::bold << "NetMessageCoder::Decode" << cc::endl;

	assert(reader.GetPosition().IsByteAligned());
	std::vector<std::shared_ptr<INetMessage>> retVal;

	while (reader.Remaining().TotalBits() >= NETMSG_TYPE_BITS)
	{
		NetMessageType type = reader.ReadInline<NetMessageType>("NetMessageCoder::NetMessageType", NETMSG_TYPE_BITS);

		if (type == NetMessageType::NET_NOOP)
		{
			if (GetBaseCmdArgs().m_PrintNet)
				cc::out << STR_FILEBITS(reader) << cc::fg::yellow << "NET_NOOP" << cc::endl;

			retVal.push_back(nullptr);
			continue;
		}

		std::shared_ptr<INetMessage> newMsg = CreateNetMessage(type);
		assert(newMsg->GetType() == type);  // Check for Bad Programmer issues

		if (GetBaseCmdArgs().m_PrintNet && (GetBaseCmdArgs().m_PrintVars || GetBaseCmdArgs().m_PrintRaw))
			cc::out << STR_FILEBITS(reader) << cc::fg::yellow << cc::bold << "Beginning read of NetMessage " << newMsg->GetType() << "..." << cc::endl;

		newMsg->ReadElement(reader);

		if (GetBaseCmdArgs().m_PrintNet)
			cc::out << STR_FILEBITS(reader) << cc::fg::yellow << *newMsg << cc::endl;

		retVal.push_back(newMsg);
	}

	return retVal;
}

void NetMessageCoder::Encode(BitIOWriter& writer, const std::vector<std::shared_ptr<INetMessage>>& messages)
{
	if (GetBaseCmdArgs().m_PrintNet)
		cc::out << cc::fg::cyan << cc::bold << "NetMessageCoder::Encode" << cc::endl;

	assert(writer.GetPosition().IsByteAligned());

	const auto startPos = writer.GetPosition();

	for (const auto& msg : messages)
	{
		if (msg)
		{
			writer.Write(msg->GetType(), NETMSG_TYPE_BITS);

			msg->WriteElement(writer);
			const auto delta = writer.GetPosition() - startPos;

			if (GetBaseCmdArgs().m_PrintNet)
				cc::out << cc::fg::magenta << delta.TotalBits() << ' ' << cc::fg::yellow << msg->GetDescription() << cc::endl;
		}
		else
		{
			writer.Write(NetMessageType::NET_NOOP, NETMSG_TYPE_BITS);

			if (GetBaseCmdArgs().m_PrintNet)
				cc::out << cc::fg::magenta << (writer.GetPosition() - startPos).TotalBits() << ' ' << cc::fg::yellow << cc::bold << "NET_NOOP" << cc::endl;
		}
	}
}

std::shared_ptr<INetMessage> NetMessageCoder::CreateNetMessage(NetMessageType type)
{
	switch (type)
	{
	case NetMessageType::NET_FILE:              return std::make_shared<NetFileMessage>();
	case NetMessageType::NET_TICK:              return std::make_shared<NetTickMessage>();
	case NetMessageType::NET_STRINGCMD:         return std::make_shared<NetStringCmdMessage>();
	case NetMessageType::NET_SETCONVAR:         return std::make_shared<NetSetConVarMessage>();
	case NetMessageType::NET_SIGNONSTATE:       return std::make_shared<NetSignonStateMessage>();
	case NetMessageType::SVC_PRINT:             return std::make_shared<NetPrintMessage>();
	case NetMessageType::SVC_SERVERINFO:        return std::make_shared<NetServerInfoMessage>();
	case NetMessageType::SVC_CLASSINFO:         return std::make_shared<NetClassInfoMessage>();
	case NetMessageType::SVC_SETPAUSE:			return std::make_shared<NetSetPauseMessage>();
	case NetMessageType::SVC_CREATESTRINGTABLE: return std::make_shared<NetCreateStringTableMessage>();
	case NetMessageType::SVC_UPDATESTRINGTABLE: return std::make_shared<NetUpdateStringTableMessage>();
	case NetMessageType::SVC_VOICEINIT:         return std::make_shared<NetVoiceInitMessage>();
	case NetMessageType::SVC_VOICEDATA:         return std::make_shared<NetVoiceDataMessage>();
	case NetMessageType::SVC_SOUND:             return std::make_shared<NetSoundMessage>();
	case NetMessageType::SVC_SETVIEW:           return std::make_shared<NetSetViewMessage>();
	case NetMessageType::SVC_FIXANGLE:          return std::make_shared<NetFixAngleMessage>();
	case NetMessageType::SVC_CROSSHAIRANGLE:	return std::make_shared<NetCrosshairAngleMessage>();
	case NetMessageType::SVC_BSPDECAL:          return std::make_shared<NetBspDecalMessage>();
	case NetMessageType::SVC_USERMESSAGE:       return std::make_shared<NetUsrMsgMessage>();
	case NetMessageType::SVC_ENTITYMESSAGE:     return std::make_shared<NetEntityMessage>();
	case NetMessageType::SVC_GAMEEVENT:         return std::make_shared<NetGameEventMessage>();
	case NetMessageType::SVC_PACKETENTITIES:    return std::make_shared<NetPacketEntitiesMessage>();
	case NetMessageType::SVC_TEMPENTITIES:      return std::make_shared<NetTempEntitiesMessage>();
	case NetMessageType::SVC_PREFETCH:          return std::make_shared<NetPrefetchMessage>();
	case NetMessageType::SVC_GAMEEVENTLIST:     return std::make_shared<NetGameEventListMessage>();
	case NetMessageType::SVC_GETCVARVALUE:      return std::make_shared<NetGetCvarValueMessage>();

	default:
		throw NotImplementedException(EnumToString(type));
	}
}