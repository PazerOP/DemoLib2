#pragma once

#include "misc/VectorXY.hpp"

#include <ostream>

class Vector final
{
public:
	Vector() = default;
	__forceinline constexpr explicit Vector(float _all) : Vector(_all, _all, _all) {}  // Broadcast
	__forceinline constexpr Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	__forceinline constexpr Vector(const VectorXY& vec) : x(vec.x), y(vec.y), z(0) {}

	__forceinline constexpr float DistToSq(const Vector& other) const
	{
		return (*this - other).LengthSq();
	}

	__forceinline float DistTo(const Vector& other) const
	{
		return (*this - other).Length();
	}

	__forceinline constexpr Vector operator-(const Vector& rhs) const
	{
		return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	__forceinline float Length() const
	{
		return std::sqrt(LengthSq());
	}

	__forceinline constexpr float LengthSq() const
	{
		return x * x + y * y + z * z;
	}

	__forceinline Vector& operator-=(const Vector& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;

		return *this;
	}
	__forceinline constexpr Vector operator*(float scalar) const
	{
		return Vector(x * scalar, y * scalar, z * scalar);
	}

	__forceinline Vector& operator=(const Vector& rhs) = default;
	__forceinline Vector& operator=(const VectorXY& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = 0;

		return *this;
	}

	__forceinline Vector& SetXY(const VectorXY& rhs)
	{
		x = rhs.x;
		y = rhs.y;

		return *this;
	}

	__forceinline Vector Round() const { return Vector(std::round(x), std::round(y), std::round(z)); }

	__forceinline float& operator[](uint_fast8_t index) { return (&x)[index]; }
	__forceinline constexpr float operator[](uint_fast8_t index) const { return (&x)[index]; }

	__forceinline constexpr bool operator==(const Vector& other) const { return x == other.x && y == other.y && z == other.z; }
	__forceinline constexpr bool operator!=(const Vector& other) const { return x != other.x || y != other.y || z != other.z; }

	float x;
	float y;
	float z;
};

static constexpr Vector vec3_nan(NAN, NAN, NAN);

inline std::ostream& operator<<(std::ostream& str, const Vector& vec)
{
	return str << vec.x << ' ' << vec.y << ' ' << vec.z;
}