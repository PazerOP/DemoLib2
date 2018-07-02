#include "NetMessageType.hpp"
#include "misc/Util.hpp"

const char* EnumToString(NetMessageType msg)
{
  switch (msg)
  {
    ENUM2STR(NetMessageType::NET_NOOP);
    ENUM2STR(NetMessageType::NET_DISCONNECT);
    ENUM2STR(NetMessageType::NET_FILE);
    ENUM2STR(NetMessageType::NET_TICK);
    ENUM2STR(NetMessageType::NET_STRINGCMD);
    ENUM2STR(NetMessageType::NET_SETCONVAR);
    ENUM2STR(NetMessageType::NET_SIGNONSTATE);
    ENUM2STR(NetMessageType::SVC_PRINT);
    ENUM2STR(NetMessageType::SVC_SERVERINFO);
    ENUM2STR(NetMessageType::SVC_SENDTABLE);
    ENUM2STR(NetMessageType::SVC_CLASSINFO);
    ENUM2STR(NetMessageType::SVC_SETPAUSE);
    ENUM2STR(NetMessageType::SVC_CREATESTRINGTABLE);
    ENUM2STR(NetMessageType::SVC_UPDATESTRINGTABLE);
    ENUM2STR(NetMessageType::SVC_VOICEINIT);
    ENUM2STR(NetMessageType::SVC_VOICEDATA);
    ENUM2STR(NetMessageType::SVC_SOUND);
    ENUM2STR(NetMessageType::SVC_SETVIEW);
    ENUM2STR(NetMessageType::SVC_FIXANGLE);
    ENUM2STR(NetMessageType::SVC_CROSSHAIRANGLE);
    ENUM2STR(NetMessageType::SVC_BSPDECAL);
    ENUM2STR(NetMessageType::SVC_USERMESSAGE);
    ENUM2STR(NetMessageType::SVC_ENTITYMESSAGE);
    ENUM2STR(NetMessageType::SVC_GAMEEVENT);
    ENUM2STR(NetMessageType::SVC_PACKETENTITIES);
    ENUM2STR(NetMessageType::SVC_TEMPENTITIES);
    ENUM2STR(NetMessageType::SVC_PREFETCH);
    ENUM2STR(NetMessageType::SVC_MENU);
    ENUM2STR(NetMessageType::SVC_GAMEEVENTLIST);
    ENUM2STR(NetMessageType::SVC_GETCVARVALUE);

    default:
      throw std::invalid_argument("msg was not a valid NetMessageType");
  }
}