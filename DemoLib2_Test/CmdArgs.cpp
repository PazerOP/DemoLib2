#include "ExtendedCmdArgs.hpp"

#include <getopt.h>
#include <iostream>
#include <string.h>

static ExtendedCmdArgs s_Args;
const BaseCmdArgs& GetBaseCmdArgs() { return s_Args; }
const ExtendedCmdArgs& GetCmdArgs() { return s_Args; }

bool ParseCmdArgs(int argc, char** argv)
{
	static const option s_Options[] =
	{
		{ "print", required_argument, nullptr, 'P' },
		{ "input", required_argument, nullptr, 'i' },
		{ "output", required_argument, nullptr, 'o' },
	};

	int longOptIndex;
	int opt;
	while ((opt = getopt_long(argc, argv, "-:i:o:P:", s_Options, &longOptIndex)) != -1)
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
				else if (!stricmp(optarg, "users"))
					s_Args.m_PrintUsers = true;
				else
				{
					cc::err << cc::fg::red << cc::bold << "Unknown print type \"" << optarg << "\" passed as -P argument" << cc::endl;
					return false;
				}
				break;
			}
			case 'i':
				s_Args.m_InFile = optarg;
				break;
			case 'o':
				s_Args.m_OutFile = optarg;
				break;

			case '?':
			{
				cc::err << cc::fg::red << cc::bold << "Unknown option ";

				if (!optopt)
					cc::err << argv[optind - 1];
				else
					cc::err << '-' << (char)optopt;

				cc::err << cc::endl;
				return false;
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