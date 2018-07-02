#include "GameEventReporter.hpp"

#include "net/data/GameEvent.hpp"
#include "net/worldstate/WorldState.hpp"

#include <iostream>

static std::ostream& operator<<(std::ostream& str, const std::variant<bool, int, float, std::string>& variant)
{
	if (auto value = std::get_if<bool>(&variant))
		return str << *value;
	if (auto value = std::get_if<int>(&variant))
		return str << *value;
	if (auto value = std::get_if<float>(&variant))
		return str << *value;
	if (auto value = std::get_if<std::string>(&variant))
		return str << *value;

	throw std::logic_error("Should never get here...");
}

GameEventReporter::GameEventReporter(const std::shared_ptr<WorldState>& world) : m_World(world)
{
	world->m_Events.AddEventListener(this);
}

GameEventReporter::~GameEventReporter()
{
	if (auto locked = m_World.lock())
		locked->m_Events.RemoveEventListener(this);
}

void GameEventReporter::GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event)
{
	cc::out << "Game event: " << event->GetDeclaration().m_Name << '\n';

	for (const auto& val : event->GetValues())
		cc::out << '\t' << val.first << ": " << val.second << '\n';

	cc::out << std::endl;
}
