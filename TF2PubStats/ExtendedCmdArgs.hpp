#pragma once

#include "interface/CmdArgs.hpp"

#include <string>

struct ExtendedCmdArgs : BaseCmdArgs
{
	ExtendedCmdArgs();

	std::string m_ConLogFile;
	std::string m_GameDir;
	std::string m_StaticAnalyzeFile;
	std::string m_StaticAnalyzeDir;
};

const ExtendedCmdArgs& GetCmdArgs();