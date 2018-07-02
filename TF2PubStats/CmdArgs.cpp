#include "ExtendedCmdArgs.hpp"

#include <getopt.h>
#include <iostream>
#include <string.h>

ExtendedCmdArgs::ExtendedCmdArgs()
{
	m_ConLogFile = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Team Fortress 2\\tf\\console.log";
	m_GameDir = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Team Fortress 2\\tf\\";
}

static ExtendedCmdArgs s_Args;
const BaseCmdArgs& GetBaseCmdArgs() { return s_Args; }
const ExtendedCmdArgs& GetCmdArgs() { return s_Args; }

bool ParseCmdArgs(int argc, char** argv)
{
	static const option s_Options[] =
	{
		{ "print", required_argument, nullptr, 'P' },
		{ "logfile", required_argument, nullptr, 'l' },
		{ "static", required_argument, nullptr, 's' },
		{ "static-dir", required_argument, nullptr, 'd'},
	};

	int longOptIndex;
	int opt;
	while ((opt = getopt_long(argc, argv, "-:l:P:s:d:", s_Options, &longOptIndex)) != -1)
	{
		switch (opt)
		{
			case 'P':
			{
				if (!stricmp(optarg, "net"))
					s_Args.m_PrintNet = true;
				else if (!stricmp(optarg, "demo"))
					s_Args.m_PrintDemo = true;
				else if (!stricmp(optarg, "vars"))
					s_Args.m_PrintVars = true;
				else if (!stricmp(optarg, "raw"))
					s_Args.m_PrintRaw = true;
				else if (!stricmp(optarg, "stringtables"))
					s_Args.m_PrintStringTables = true;
				else
				{
					cc::err << cc::fg::red << cc::bold << "Unknown print type \"" << optarg << "\" passed as -P argument" << cc::endl;
					return false;
				}
				break;
			}
			case 'l':
				s_Args.m_ConLogFile = optarg;
				break;

			case 's':
				s_Args.m_StaticAnalyzeFile = optarg;
				break;

			case 'd':
			{
				s_Args.m_StaticAnalyzeDir = optarg;
				break;
			}

			case '?':
			{
				cc::err << cc::fg::red << cc::bold << "Unknown option ";

				if (!optopt)
					cc::err << argv[optind - 1];
				else
					cc::err << '-' << (char)optopt;

				cc::err << cc::endl;
				break;// return false;
			}
			case ':':
			{
				cc::err << cc::fg::red << cc::bold << "Missing argument to option " << argv[optind - 1] << cc::endl;
				return false;
			}

			default:
				cc::err << cc::fg::red << cc::bold << "getopt_long returned unexpected value " <<
					opt << " (char " << (char)opt << ')' << cc::endl;
				return false;
		}
	}

	return true;
}