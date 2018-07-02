#pragma once

#include "misc/Breakpoints.hpp"
#include "misc/CompilerSettings.hpp"
#include "misc/ShorthandIntTypes.hpp"

#include <cinttypes>
#include <iosfwd>

#ifdef __GNUC__
#define stricmp strcasecmp
#endif

#ifdef __OPTIMIZE__
#define RELEASE __OPTIMIZE__
#else
#define DEBUG 1
#endif