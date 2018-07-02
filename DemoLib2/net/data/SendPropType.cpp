#include "SendPropType.hpp"
#include "misc/Util.hpp"

const char* EnumToString(SendPropType msg)
{
  switch (msg)
  {
    ENUM2STR(SendPropType::Int);
    ENUM2STR(SendPropType::Float);
    ENUM2STR(SendPropType::Vector);
    ENUM2STR(SendPropType::VectorXY);
    ENUM2STR(SendPropType::String);
    ENUM2STR(SendPropType::Array);
    ENUM2STR(SendPropType::Datatable);

    default:
      throw std::invalid_argument("msg was not a valid SendPropType");
  }
}