#pragma once

struct BaseCmdArgs
{
	bool m_PrintRaw = false;
	bool m_PrintNet = false;
	bool m_PrintDemo = false;
	bool m_PrintVars = false;
	bool m_PrintStringTables = false;
};

bool ParseCmdArgs(int argc, char** argv);
const BaseCmdArgs& GetBaseCmdArgs();