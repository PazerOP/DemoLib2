#include "Util.hpp"

#include <cassert>
#include <cstdarg>

std::string strprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	std::string retVal;

	// Make sure we have enough storage
	size_t charCount;
	{
		va_list args2;
		va_copy(args2, args);
		charCount = vsnprintf(nullptr, 0, fmt, args2);
		retVal.resize(charCount + 1);
		va_end(args2);
	}

	// Write into the string
	[[maybe_unused]] auto result = vsnprintf(retVal.data(), charCount + 1, fmt, args);
	assert(result >= 0);

	va_end(args);
	return retVal;
}