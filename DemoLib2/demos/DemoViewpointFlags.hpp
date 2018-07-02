#pragma once

#include <cstdint>

enum class DemoViewpointFlags : int32_t
{
	None = 0,
	UseOrigin2 = (1 << 0),
	UseAngles2 = (1 << 1),
	NoInterp = (1 << 2),
};