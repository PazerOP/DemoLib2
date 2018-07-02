#pragma once

#include <ostream>

template<typename T = char>
class StrBuf : public std::basic_iostream<T>, std::streambuf
{
public:
	StrBuf(char* buffer, size_t bufElemCount, size_t curPos = 0) : std::basic_iostream<T>(this)
	{
		setp(buffer, buffer + curPos, buffer + bufElemCount);
	}
	StrBuf(StrBuf&& other) : std::basic_iostream<T>(this)
	{
		setp(other.pbase(), other.pptr(), other.epptr());
		other.setp(nullptr, nullptr);
	}
	~StrBuf()
	{
		// Null terminate
		if (pptr() < epptr())
			*pptr() = 0;
		else if (pptr() - 1 >= pbase())
			*(pptr() - 1) = 0;
	}

	std::string_view str() const { return std::string_view(pbase(), pptr() - pbase()); }

protected:
	int underflow() override
	{
		setg(pbase(), gptr() ? gptr() : pbase(), epptr());

		if (gptr() == epptr())
			return traits_type::eof();
		else
			return *gptr();
	}
};

template<size_t size, typename T = char>
class StaticStrBuf : public StrBuf<T>
{
public:
	StaticStrBuf() : std::basic_iostream<T>(this)
	{
		setp(m_Buffer, m_Buffer, m_Buffer + size);
	}

private:
	char m_Buffer[size];
};

template<typename T> std::ostream& operator<<(std::ostream& os, const StrBuf<T>& buf)
{
	return os << buf.str();
}

template<size_t size, typename T> StrBuf<T> make_strbuf(T(&array)[size])
{
	return StrBuf<T>(array, size);
}