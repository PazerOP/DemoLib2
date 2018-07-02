#pragma once

#include "BitIO/IStreamElement.hpp"

class WorldState;

class IGameStreamElement : public IStreamElement
{
public:
	virtual ~IGameStreamElement() = default;

	virtual void ApplyWorldState(WorldState& world) const = 0;
};