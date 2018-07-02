#pragma once

#include <cstdint>

enum class FileStatus : uint_fast8_t
{
  Denied = 0,
  Requested = 1,
};