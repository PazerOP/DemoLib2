#pragma once

#include <ostream>

class VectorXY final
{
public:
  VectorXY() = default;
  explicit VectorXY(float _all) : VectorXY(_all, _all) { }  // Broadcast
  VectorXY(float _x, float _y)
  {
    x = _x;
    y = _y;
  }

  float x;
  float y;
};

inline std::ostream& operator<<(std::ostream& str, const VectorXY& vec)
{
  return str << vec.x << ' ' << vec.y;
}