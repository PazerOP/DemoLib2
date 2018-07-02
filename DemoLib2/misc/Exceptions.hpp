#pragma once

#include <stdexcept>
#include <string>

using namespace std::string_literals;

class NotImplementedException : public std::logic_error
{
public:
	NotImplementedException() : std::logic_error("not implemented") {}
	NotImplementedException(const char* details) : std::logic_error("not implemented: "s + details) {}
};

class ParseException : public std::runtime_error
{
public:
	ParseException(const char* details) : std::runtime_error(details) {}
	ParseException(const std::string& details) : std::runtime_error(details) {}
};