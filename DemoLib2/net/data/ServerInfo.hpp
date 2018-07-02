#pragma once

#include "net/data/OperatingSystem.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

struct ServerInfo
{
	uint16_t m_Protocol;          // Protocol version
	uint32_t m_ServerCount;       // Number of changelevels since server start
	bool m_IsHLTV;
	bool m_IsDedicated;           // Dedicated server?
	uint32_t m_ClientCRC;         // CRC32 of client.dll
	uint16_t m_MaxClasses;        // Max number of server classes
	std::byte m_MapMD5[16];       // Map MD5 sum
	uint8_t m_PlayerSlot;         // Our client slot number
	uint8_t m_MaxClients;
	float m_TickInterval;         // Server tick interval
	OperatingSystem m_OS;
	std::string m_GameDirectory;  // Game directory, eg "tf"
	std::string m_MapName;
	std::string m_SkyName;        // Current skybox name
	std::string m_Hostname;       // Server name
	bool _unknown;
};