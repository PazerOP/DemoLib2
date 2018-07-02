#include "GameEventDataType.hpp"
#include "misc/Util.hpp"

const char* EnumToString(GameEventDataType type)
{
  switch (type)
  {
    ENUM2STR(GameEventDataType::Local);
    ENUM2STR(GameEventDataType::String);
    ENUM2STR(GameEventDataType::Float);
    ENUM2STR(GameEventDataType::Long);
    ENUM2STR(GameEventDataType::Short);
    ENUM2STR(GameEventDataType::Byte);
    ENUM2STR(GameEventDataType::Bool);

    default:
      throw std::invalid_argument("type was not a valid GameEventDataType");
  }
}