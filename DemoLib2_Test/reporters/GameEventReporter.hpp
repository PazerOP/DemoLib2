#pragma once

#include "net/worldstate/IWorldEventListener.hpp"

#include <vector>

class GameEventReporter final : IWorldEventListener
{
public:
	GameEventReporter(const std::shared_ptr<WorldState>& world);
	~GameEventReporter();

protected:
	void GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event) override;

private:
	std::weak_ptr<WorldState> m_World;
};