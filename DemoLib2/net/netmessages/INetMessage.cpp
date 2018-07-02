#include "INetMessage.hpp"

#include <ostream>
#include <sstream>
#include <string>

std::string INetMessage::GetDescription() const
{
	std::stringstream retVal;
	GetDescription(retVal);
	return retVal.str();
}