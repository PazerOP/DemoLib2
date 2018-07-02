#include "CommonConsoleStream.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>

#include <Windows.h>

#include <wincon.h>

#undef min
#undef max

//#define OUTPUT_TO_DEBUGGER 1

namespace cc
{
	int CommonConsoleStream::s_DefaultAttributes;
	std::once_flag CommonConsoleStream::s_DefaultAttributesFlag;

	static std::mutex s_OutputMutex;

	// Common console stream objects
	thread_local CommonConsoleStream out(stdout);
	thread_local CommonConsoleStream err(stderr);
	thread_local DebuggerStream dbg;

	enum class BaseColor
	{
		Black,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,
	};

#ifdef _WIN32
	static void SetConsoleTextAttributes(int attributes, int mask)
	{
		auto conHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO cInfo;
		if (GetConsoleScreenBufferInfo(conHandle, &cInfo) != TRUE)
			throw std::runtime_error("Failed to GetConsoleScreenBufferInfo");

		cInfo.wAttributes &= ~mask;
		cInfo.wAttributes |= attributes;

		// Actually apply the color to the CSB
		if (SetConsoleTextAttribute(conHandle, cInfo.wAttributes) != TRUE)
			throw std::runtime_error("Failed to SetConsoleTextAttribute");
	}

	static void ShiftCursor(int delta, bool x)
	{
		auto conHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO cInfo;
		if (GetConsoleScreenBufferInfo(conHandle, &cInfo) != TRUE)
			throw std::runtime_error("Failed to GetConsoleScreenBufferInfo");

		if (x)
			cInfo.dwCursorPosition.X += delta;
		else
			cInfo.dwCursorPosition.Y += delta;

		if (SetConsoleCursorPosition(conHandle, cInfo.dwCursorPosition) != TRUE)
			throw std::runtime_error("Failed to SetConsoleCursorPosition");
	}

	static void SetCursorPos(int x, int y)
	{
		auto conHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO cInfo;
		if (GetConsoleScreenBufferInfo(conHandle, &cInfo) != TRUE)
			throw std::runtime_error("Failed to GetConsoleScreenBufferInfo");

		cInfo.dwCursorPosition.X = x;
		cInfo.dwCursorPosition.Y = y;

		if (SetConsoleCursorPosition(conHandle, cInfo.dwCursorPosition) != TRUE)
			throw std::runtime_error("Failed to SetConsoleCursorPosition");
	}

	static void ClearScreen()
	{
#if 1
		// Since this code is windows-specific
		system("cls");
#else
		auto conHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO cInfo;
		if (GetConsoleScreenBufferInfo(conHandle, &cInfo) != TRUE)
			throw std::runtime_error("Failed to GetConsoleScreenBufferInfo");

		if (!FillConsoleOutputCharacterA(conHandle, ' ', cInfo.dwSize.X * cInfo.dwSize.Y, COORD{ 0, 0 }, nullptr))
			throw std::runtime_error("Failed to FillConsoleOutputCharacter");

		if (SetConsoleCursorPosition(conHandle, COORD{ 0, 0 }) != TRUE)
			throw std::runtime_error("Failed to SetConsoleCursorPosition");
#endif
	}
	static void ClearLine()
	{
		auto conHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO cInfo;
		if (GetConsoleScreenBufferInfo(conHandle, &cInfo) != TRUE)
			throw std::runtime_error("Failed to GetConsoleScreenBufferInfo");

		DWORD dummy;
		if (!FillConsoleOutputCharacterA(conHandle, ' ', cInfo.dwSize.X - cInfo.dwCursorPosition.X, cInfo.dwCursorPosition, &dummy))
			throw std::runtime_error("Failed to FillConsoleOutputCharacter");

		//if (SetConsoleCursorPosition(conHandle, COORD{ 0, 0 }) != TRUE)
		//	throw std::runtime_error("Failed to SetConsoleCursorPosition");
	}

	static void SetConsoleColor(BaseColor color_for, bool bright, bool background)
	{
		int colors = 0;

		if (bright)
			colors |= FOREGROUND_INTENSITY;

		// Apply color
		{
			static constexpr int s_BaseConsoleColorMap[8] =
			{
				0,
				FOREGROUND_RED,                                         // Red
				FOREGROUND_GREEN,                                       // Green
				FOREGROUND_RED | FOREGROUND_GREEN,                      // Yellow
				FOREGROUND_BLUE,                                        // Blue
				FOREGROUND_RED | FOREGROUND_BLUE,                       // Magenta
				FOREGROUND_GREEN | FOREGROUND_BLUE,                     // Cyan
				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,    // White
			};

			colors |= s_BaseConsoleColorMap[(int)color_for];
		}

		const auto shift = background ? 4 : 0;
		SetConsoleTextAttributes(colors << shift, 0xF << shift);
	}
#endif

	CommonConsoleStream::CommonConsoleStream(FILE* stream) : std::ostream(this), m_Stream(stream)
	{
		std::call_once(s_DefaultAttributesFlag, []()
		{
			CONSOLE_SCREEN_BUFFER_INFO cInfo;
			if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo) != TRUE)
				throw std::runtime_error("Failed to GetConsoleScreenBufferInfo");

			s_DefaultAttributes = cInfo.wAttributes;
		});

		//static_cast<std::streambuf*>(this)->setp(m_Buffer, std::size(m_Buffer));

		//if (SetConsoleCP(CP_UTF8) != TRUE)
		//	std::cout << "Failed to set console codepage to UTF-8" << std::endl;
		//if (SetConsoleOutputCP(CP_UTF8) != TRUE)
		//	std::cout << "Failed to set console output codepage to UTF-8" << std::endl;
	}

	int CommonConsoleStream::sync()
	{
		std::lock_guard<decltype(s_OutputMutex)> lock(s_OutputMutex);

		const auto dbg = std::string_view(pbase(), pptr() - pbase());
		const size_t count = pptr() - pbase();
		const auto bufSize = count + 1;

		char escSeqBuf[256];
		uint_fast8_t escSeqBufPos = 0;
		char* unwrittenBegin = pbase();       // Last character that was parsed that was NOT part of an escape sequence
		char* unwrittenEnd = pbase();    // Last non-escape-sequence character that was written to the output stream

		bool inEscSeq = false;
		for (char* c = pbase(); c != pptr(); c++)
		{
			if (*c == '\x1B' && !inEscSeq)
			{
				inEscSeq = true;

				if (unwrittenBegin < unwrittenEnd)
					WriteAndFlush(unwrittenBegin, unwrittenEnd - unwrittenBegin);
			}
			else if (inEscSeq)
			{
				switch (*c)
				{
					case 'm':
						escSeqBuf[escSeqBufPos++] = '\0';
						ApplyEscapeSequence(std::string_view(escSeqBuf, escSeqBufPos));
						escSeqBufPos = 0;
						inEscSeq = false;
						unwrittenBegin = c + 1;
						break;

					default:
						assert(escSeqBufPos < std::size(escSeqBuf));
						escSeqBuf[escSeqBufPos++] = *c;    // Continue gather
				}
			}
			else
			{
				unwrittenEnd = c + 1;
			}
		}

		if (unwrittenBegin < unwrittenEnd)
			WriteAndFlush(unwrittenBegin, unwrittenEnd - unwrittenBegin);

		pubseekpos(0, out);
		return 0;
	}

	void CommonConsoleStream::ApplyEscapeSequence(const std::string_view& escSeq)
	{
#ifdef _WIN32
		char test = '\x1B';

		int pos;
		{
			int r, g, b;
			if (sscanf_s(escSeq.data(), "[38;2;%i;%i;%i%n", &r, &g, &b, &pos) == 3 && pos == (escSeq.size() - 1))
			{
				// Not supported on windows
				return;
			}
		}
		if (!strcmp(escSeq.data(), "[K"))
		{
			ClearLine();
			return;
		}
		if (!strcmp(escSeq.data(), "[2J"))
		{
			ClearScreen();
			return;
		}
		{
			int delta;
			char dir;
			if (sscanf_s(escSeq.data(), "[%i%1c%n", &delta, &dir, 1, &pos) == 2 && pos == (escSeq.size() - 1))
			{
				switch (dir)
				{
					case 'A':
						ShiftCursor(delta, false);
						break;
					case 'B':
						ShiftCursor(-delta, false);
						break;
					case 'C':
						ShiftCursor(delta, true);
						break;
					case 'D':
						ShiftCursor(-delta, true);
						break;

					default:
						assert(false);
						throw std::invalid_argument("Unknown direction for \"move cursor\" operation");
				}

				return;
			}
		}
		if (int x, y; (sscanf_s(escSeq.data(), "[%i;%iH%n", &x, &y, &pos) == 2 || sscanf_s(escSeq.data(), "[%i;%if%n", &x, &y, &pos) == 2) && pos == (escSeq.size() - 1))
		{
			SetCursorPos(x, y);

			return;
		}
		if (int baseSetting; sscanf_s(escSeq.data(), "[%i%n", &baseSetting, &pos) == 1 && pos == (escSeq.size() - 1))
		{
			if (baseSetting >= 30 && baseSetting <= 37)
				SetConsoleColor(BaseColor(baseSetting - 30), false, false);
			else if (baseSetting >= 40 && baseSetting <= 47)
				SetConsoleColor(BaseColor(baseSetting - 40), false, true);
			else if (baseSetting >= 90 && baseSetting <= 97)
				SetConsoleColor(BaseColor(baseSetting - 90), true, false);
			else if (baseSetting >= 100 && baseSetting <= 107)
				SetConsoleColor(BaseColor(baseSetting - 100), true, true);
			else
			{
				switch (baseSetting)
				{
					case 0:  // Reset
						SetConsoleTextAttributes(s_DefaultAttributes, 0xFFFFFFFF);
						break;

					case 1:  // Bold
					case 21: // Unbold
						break;  // Not supported on Windows

					case 4:  // Underline
						SetConsoleTextAttributes(COMMON_LVB_UNDERSCORE, COMMON_LVB_UNDERSCORE);
						break;
					case 24: // Un-underline
						SetConsoleTextAttributes(0, COMMON_LVB_UNDERSCORE);
						break;

					case 7:
						SetConsoleTextAttributes(COMMON_LVB_REVERSE_VIDEO, COMMON_LVB_REVERSE_VIDEO);
						break;
					case 27: // Uninvert
						SetConsoleTextAttributes(0, COMMON_LVB_REVERSE_VIDEO);

					default:
						assert(false);
						throw std::invalid_argument("Unknown base setting");
				}
			}


			return;
		}

		assert(false);
		throw std::invalid_argument("Unknown escape sequence");

#else
		static_assert(false, "Unknown/unsupported platform");
#endif
	}

	void CommonConsoleStream::WriteAndFlush(const char* str, size_t count)
	{
		auto dbg = std::string_view(str, count);
		auto written = fwrite(str, sizeof(*str), count, m_Stream);
		assert(written == count);

		if (fflush(m_Stream) != 0)
			throw std::runtime_error("Failed to flush stream");
	}

	DebuggerStream::DebuggerStream() : std::ostream(this)
	{
	}

	int DebuggerStream::sync()
	{
		std::lock_guard<decltype(s_OutputMutex)> lock(s_OutputMutex);

		const size_t count = pptr() - pbase();
		const auto bufSize = count + 1;

		char* const buf = (char*)_alloca(bufSize);
		char* bufPos = buf;

		bool inEscSeq = false;
		for (char* c = pbase(); c != pptr(); c++)
		{
			if (*c == '\x1B' && !inEscSeq)
				inEscSeq = true;
			else if (inEscSeq)
			{
				if (*c == 'm')
					inEscSeq = false;
			}
			else
				*(bufPos++) = *c;
		}

		*(bufPos++) = '\0';

		OutputDebugStringA(buf);
		pubseekpos(0, out);
		return 0;
	}
}