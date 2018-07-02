#pragma once

#include "misc/Vector.hpp"
#include "misc/VectorXY.hpp"

#include <variant>
#include <vector>

using SendPropVariant = std::variant<int32_t, float, Vector, VectorXY, std::string, std::vector<struct SendPropArrayElement>>;

struct SendPropArrayElement : public SendPropVariant
{
	SendPropArrayElement(const SendPropVariant& variant) : SendPropVariant(variant) {}
	SendPropArrayElement(SendPropVariant&& variant) : SendPropVariant(variant) {}
};