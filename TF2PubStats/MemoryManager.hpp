#pragma once

#include <stdexcept>

class MemoryException : public std::runtime_error
{
public:
	MemoryException(const char* description) : std::runtime_error(nullptr), m_Description(description) {}

	const char* what() const override { return m_Description; }

private:
	const char* m_Description;
};