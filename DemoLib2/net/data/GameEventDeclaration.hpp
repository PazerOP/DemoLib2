#pragma once

#include "net/data/GameEventDataType.hpp"

#include <string>
#include <utility>
#include <vector>

struct GameEventDeclaration
{
	uint32_t m_ID;
	std::string m_Name;
	std::vector<std::pair<std::string, GameEventDataType>> m_Values;
};