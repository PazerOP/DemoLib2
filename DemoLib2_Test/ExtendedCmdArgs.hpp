#pragma once

#include "interface/CmdArgs.hpp"

#include <string>

struct ExtendedCmdArgs : BaseCmdArgs
{
	std::string m_InFile;
	std::string m_OutFile;

	bool m_PrintUsers;
};

const ExtendedCmdArgs& GetCmdArgs();