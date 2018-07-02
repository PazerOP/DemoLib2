#pragma once

#include <mutex>
#include <ostream>
#include <sstream>
#include <streambuf>

namespace cc
{
	class CommonConsoleStream : std::stringbuf, public std::ostream
	{
	public:
		CommonConsoleStream(FILE* stream);

	protected:
		int sync() override;

	private:
		void ApplyEscapeSequence(const std::string_view& escSeq);
		void WriteAndFlush(const char* str, size_t count);

		FILE* m_Stream;
		std::string m_DebuggerString;

		static int s_DefaultAttributes;
		static std::once_flag s_DefaultAttributesFlag;
	};

	class DebuggerStream : std::stringbuf, public std::ostream
	{
	public:
		DebuggerStream();

	protected:
		int sync() override;
	};

	extern thread_local CommonConsoleStream out;
	extern thread_local CommonConsoleStream err;
	extern thread_local DebuggerStream dbg;

#define OSTR_MANIP(name) template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& name(std::basic_ostream<_Elem, _Traits>& os)

	OSTR_MANIP(reset)
	{
		return os << "\x1B[0m";
	}

	// cc::reset + std::endl;
	OSTR_MANIP(endl)
	{
		return os << reset << '\n' << std::flush;
	}

	OSTR_MANIP(bold)
	{
		return os << "\x1B[1m";
	}
	OSTR_MANIP(boldoff)
	{
		return os << "\x1B[21m";
	}
	OSTR_MANIP(ul)
	{
		return os << "\x1B[4m";
	}
	OSTR_MANIP(uloff)
	{
		return os << "\x1B[24m";
	}
	OSTR_MANIP(invert)
	{
		return os << "\x1B[7m";
	}
	OSTR_MANIP(invertoff)
	{
		return os << "\x1B[27m";
	}

	namespace impl
	{
		struct rgb
		{
			constexpr rgb(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b) {}
			constexpr rgb(uint32_t _rgb) : r((_rgb >> 4) & 0xFF), g((_rgb >> 2) & 0xFF), b(_rgb & 0xFF) {}

			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
	}

	namespace fg
	{
		struct rgb : impl::rgb
		{
			constexpr rgb(uint8_t _r, uint8_t _g, uint8_t _b) : impl::rgb(_r, _g, _b) {}
			constexpr rgb(uint32_t _rgb) : impl::rgb(_rgb) {}
		};

		OSTR_MANIP(black)
		{
			return os << "\x1B[30m";
		}
		OSTR_MANIP(red)
		{
			return os << "\x1B[91m";
		}
		OSTR_MANIP(green)
		{
			return os << "\x1B[92m";
		}
		OSTR_MANIP(yellow)
		{
			return os << "\x1B[93m";
		}
		OSTR_MANIP(blue)
		{
			return os << "\x1B[94m";
		}
		OSTR_MANIP(magenta)
		{
			return os << "\x1B[95m";
		}
		OSTR_MANIP(cyan)
		{
			return os << "\x1B[96m";
		}
		OSTR_MANIP(white)
		{
			return os << "\x1B[97m";
		}
		OSTR_MANIP(default_)
		{
			return os << "\x1B[39m";
		}
		OSTR_MANIP(grey)
		{
			return os << "\x1B[37m";    // "Dark white"
		}

		namespace dark
		{
			OSTR_MANIP(red)
			{
				return os << "\x1B[31m";
			}
			OSTR_MANIP(green)
			{
				return os << "\x1B[32m";
			}
			OSTR_MANIP(yellow)
			{
				return os << "\x1B[33m";
			}
			OSTR_MANIP(blue)
			{
				return os << "\x1B[34m";
			}
			OSTR_MANIP(magenta)
			{
				return os << "\x1B[35m";
			}
			OSTR_MANIP(cyan)
			{
				return os << "\x1B[36m";
			}
			OSTR_MANIP(grey)
			{
				return os << "\x1B[90m";    // "Bright black"
			}
		}
	}
	namespace bg
	{
		struct rgb : impl::rgb
		{
			constexpr rgb(uint8_t _r, uint8_t _g, uint8_t _b) : impl::rgb(_r, _g, _b) {}
			constexpr rgb(uint32_t _rgb) : impl::rgb(_rgb) {}
		};

		OSTR_MANIP(black)
		{
			return os << "\x1B[40m";
		}
		OSTR_MANIP(red)
		{
			return os << "\x1B[101m";
		}
		OSTR_MANIP(green)
		{
			return os << "\x1B[102m";
		}
		OSTR_MANIP(yellow)
		{
			return os << "\x1B[103m";
		}
		OSTR_MANIP(blue)
		{
			return os << "\x1B[104m";
		}
		OSTR_MANIP(magenta)
		{
			return os << "\x1B[105m";
		}
		OSTR_MANIP(cyan)
		{
			return os << "\x1B[106m";
		}
		OSTR_MANIP(white)
		{
			return os << "\x1B[107m";
		}
		OSTR_MANIP(grey)
		{
			return os << "\x1B[47m";    // "Dark white"
		}
		OSTR_MANIP(default_)
		{
			return os << "\x1B[49m";
		}

		namespace dark
		{
			OSTR_MANIP(red)
			{
				return os << "\x1B[41m";
			}
			OSTR_MANIP(green)
			{
				return os << "\x1B[42m";
			}
			OSTR_MANIP(yellow)
			{
				return os << "\x1B[43m";
			}
			OSTR_MANIP(blue)
			{
				return os << "\x1B[44m";
			}
			OSTR_MANIP(magenta)
			{
				return os << "\x1B[45m";
			}
			OSTR_MANIP(cyan)
			{
				return os << "\x1B[46m";
			}
			OSTR_MANIP(grey)
			{
				return os << "\x1B[100m";    // "Bright black"
			}
		}
	}

	namespace clear
	{
		OSTR_MANIP(screen)
		{
			return os << "\x1B[2Jm";
		}
		OSTR_MANIP(line)
		{
			return os << "\x1B[Km";
		}
	}

	namespace impl
	{
		struct move
		{
			constexpr move(int delta) : m_Delta(delta) {}
			int m_Delta;
		};
	}

	namespace curpos
	{
		struct moveleft : impl::move { constexpr moveleft(int delta) : move(delta) {} };
		struct moveright : impl::move { constexpr moveright(int delta) : move(delta) {} };
		struct moveup : impl::move { constexpr moveup(int delta) : move(delta) {} };
		struct movedown : impl::move { constexpr movedown(int delta) : move(delta) {} };

		struct set
		{
			constexpr set(uint32_t _x, uint32_t _y) : x(_x), y(_y) {}

			uint32_t x;
			uint32_t y;
		};

		OSTR_MANIP(save)
		{
			return os << "\x1B[sm";
		}
		OSTR_MANIP(restore)
		{
			return os << "\x1B[um";
		}
	}

#undef OSTR_MANIP
}
template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::fg::rgb& rgb)
{
	return os << "\x1B[38;2;" << +rgb.r << ';' << +rgb.g << ';' << +rgb.b << 'm';
}
template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::bg::rgb& rgb)
{
	return os << "\x1B[48;2;" << +rgb.r << ';' << +rgb.g << ';' << +rgb.b << 'm';
}

template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::curpos::moveup& up)
{
	return os << "\x1B[" << up.m_Delta << "Am";
}
template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::curpos::movedown& down)
{
	return os << "\x1B[" << down.m_Delta << "Bm";
}
template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::curpos::moveright& right)
{
	return os << "\x1B[" << right.m_Delta << "Cm";
}
template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::curpos::moveleft& left)
{
	return os << "\x1B[" << left.m_Delta << "Dm";
}
template<class _Elem, class _Traits> inline std::basic_ostream<_Elem, _Traits>& operator<<(std::basic_ostream<_Elem, _Traits>& os, const cc::curpos::set& set)
{
	return os << "\x1B[" << set.x << ';' << set.y << "Hm";
}