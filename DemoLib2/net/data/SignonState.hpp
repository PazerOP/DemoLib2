#pragma once

#include "net/data/ConnectionState.hpp"

struct SignonState
{
	ConnectionState m_State;
	uint32_t m_SpawnCount;

	bool operator==(const SignonState& other) const
	{
		return (m_State == other.m_State &&
			m_SpawnCount == other.m_SpawnCount);
	}
	bool operator!=(const SignonState& other) const { return !operator==(other); }
};