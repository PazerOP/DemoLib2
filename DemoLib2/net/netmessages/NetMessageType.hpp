#pragma once

#include <cstdint>
#include <ostream>

enum class NetMessageType : uint_fast8_t
{
	NET_NOOP = 0,
	NET_DISCONNECT = 1,
	NET_FILE = 2,
	NET_TICK = 3,
	NET_STRINGCMD = 4,
	NET_SETCONVAR = 5,
	NET_SIGNONSTATE = 6,
	SVC_PRINT = 7,
	SVC_SERVERINFO = 8,
	SVC_SENDTABLE = 9,
	SVC_CLASSINFO = 10,
	SVC_SETPAUSE = 11,
	SVC_CREATESTRINGTABLE = 12,
	SVC_UPDATESTRINGTABLE = 13,
	SVC_VOICEINIT = 14,
	SVC_VOICEDATA = 15,
	// 16
	SVC_SOUND = 17,
	SVC_SETVIEW = 18,
	SVC_FIXANGLE = 19,
	SVC_CROSSHAIRANGLE = 20,
	SVC_BSPDECAL = 21,
	// 22
	SVC_USERMESSAGE = 23,
	SVC_ENTITYMESSAGE = 24,
	SVC_GAMEEVENT = 25,
	SVC_PACKETENTITIES = 26,
	SVC_TEMPENTITIES = 27,
	SVC_PREFETCH = 28,
	SVC_MENU = 29,
	SVC_GAMEEVENTLIST = 30,
	SVC_GETCVARVALUE = 31,
};

const char* EnumToString(NetMessageType msg);

inline std::ostream& operator<<(std::ostream& os, NetMessageType cmd)
{
	return os << EnumToString(cmd);
}