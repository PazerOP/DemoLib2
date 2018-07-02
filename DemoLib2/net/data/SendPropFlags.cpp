#include "SendPropFlags.hpp"
#include "misc/Util.hpp"

static constexpr const char* s_FlagBitNames[] = 
{
  "Unsigned",             // 0
  "Coord",                // 1
  "NoScale",              // 2
  "RoundDown",            // 3
  "RoundUp",              // 4
  "Normal/VarInt",        // 5
  "Exclude",              // 6
  "EncodeXYZE",           // 7
  "InsideArray",          // 8
  "ProxyAlwaysYes",       // 9
  "ChangesOften",         // 10
  "IsVectorElement",      // 11
  "Collapsible",         // 12
  "CoordMP",              // 13
  "CoordMPLowPrecision",  // 14
  "CoordMPIntegral",      // 15
};

std::string EnumToString(SendPropFlags flags)
{
  std::string retVal;

  for (uint_fast8_t i = 0; i < std::size(s_FlagBitNames); i++)
  {
    if (!(std::underlying_type_t<SendPropFlags>(flags) & (1 << i)))
      continue;

    const char* current = s_FlagBitNames[i];

    if (current)
    {
      if (retVal.size() > 0)
        retVal += " | ";
      
      retVal += current;
    }
  }

  if (!retVal.size())
    retVal = "None";

  return retVal;
}