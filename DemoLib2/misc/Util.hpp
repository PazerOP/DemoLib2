#pragma once
#include <cinttypes>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>

#define _M_xstr(a) _M_str(a)
#define _M_str(a) #a

#define ENUM2STR(val) \
  case val: return _M_xstr(val)

#define STR_FILEBITS(reader) cc::reset << cc::fg::magenta << (reader).GetPosition().TotalBits() << ' '

#ifdef __GNUC__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

using namespace std::string_literals;

template<class T> constexpr T IntCeil(const T& x, const T& y)
{
	return (x / y) + (x % y != 0);
}

#ifdef __GNUC__
#define __forceinline inline __attribute__((always_inline))
#endif

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
__forceinline constexpr uint_fast8_t Log2(T input)
{
	uint_fast8_t retVal = 0;
	static_assert(sizeof(T) * 8 < std::numeric_limits<decltype(retVal)>::max());

	while ((input >>= 1) > 0)
		retVal++;

	return retVal;
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
__forceinline constexpr uint_fast8_t Log2Ceil(T input)
{
	uint_fast8_t retVal = 0;
	static_assert(sizeof(T) * 8 < std::numeric_limits<decltype(retVal)>::max());

	while (input > 0)
	{
		input >>= 1;
		retVal++;
	}

	return retVal;
}

template<typename T>
__forceinline std::remove_reference_t<std::remove_const_t<T>> copy(T val)
{
	return val;
}

// Allows promoting characters to printable integers in templates in BitIOReader.
template<class T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr T operator+(const T& a) { return a; }

template<class T> std::ostream& operator<<(std::ostream& os, const std::optional<T>& data)
{
	if (data.has_value())
		return os << data.value();
	return
		os << "(null)";
}

template<class T> std::string tostr(const T& input)
{
	std::stringstream stream;
	stream << input;
	return stream.str();
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr uint_fast8_t MinBits(T input)
{
	uint_fast8_t retVal = !!input;
	while ((input >>= 1) > 0)
		retVal++;
	return retVal;
}

static_assert(MinBits(0) == 0);
static_assert(MinBits(1) == 1);
static_assert(MinBits(3) == 2);
static_assert(MinBits(4) == 3);
static_assert(MinBits(7) == 3);

inline const std::string& strfmt(std::string&& fmt) { return fmt; }
template<class... Args> std::string&& strfmt(std::string&& fmt, const Args&... args)
{
	const std::string strArgs[] = { tostr(args)... };
	constexpr size_t arraySize = sizeof...(Args);

	for (size_t i = 0; i < arraySize; i++)
	{
		char arg[16];
		const auto argLength = snprintf(arg, sizeof(arg), "{%zu}", i);

		std::string::size_type found;
		while ((found = fmt.find(arg)) != std::string::npos)
			fmt.replace(found, argLength, strArgs[i]);
	}

	return std::move(fmt);
}

