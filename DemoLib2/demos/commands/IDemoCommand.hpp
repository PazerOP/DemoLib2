#pragma once
#include "demos/DemoCommandType.hpp"
#include "net/data/IGameStreamElement.hpp"

class IDemoCommand : public IGameStreamElement
{
public:
	virtual ~IDemoCommand() = default;

	virtual DemoCommandType GetType() const = 0;
};