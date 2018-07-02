#pragma once

#include <cstdint>

enum class ConnectionState : uint_fast8_t
{
  // No state yet, about to connect
  None = 0,

  // Client challenging server, all OOB packets
  Challenge = 1,

  // Client is connected to server, netchannels ready
  Connected = 2,

  // Just got serverinfo and stringtables
  New = 3,

  // Received signon buffers
  Prespawn = 4,

  // Ready to receive entity packets
  Spawn = 5,

  // We are fully connected, first non-delta packet received
  Full = 6,

  // Server is changing level, please wait
  Changelevel = 7,
};