#include "DemoCommandType.hpp"
#include "misc/Util.hpp"

#include <stdexcept>

const char* EnumToString(DemoCommandType cmd)
{
	switch (cmd)
	{
		ENUM2STR(dem_invalid);
		ENUM2STR(dem_signon);
		ENUM2STR(dem_packet);
		ENUM2STR(dem_synctick);
		ENUM2STR(dem_consolecmd);
		ENUM2STR(dem_usercmd);
		ENUM2STR(dem_datatables);
		ENUM2STR(dem_stop);
		ENUM2STR(dem_stringtables);

		default:
			throw std::invalid_argument("cmd was not a valid DemoCommandType");
	}
}