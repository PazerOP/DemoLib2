#include "WorldEvents.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>

template<typename fn>
bool WorldEvents::CallPreFunc(const fn& func)
{
	for (auto listener : m_EventListeners)
	{
		if (!func(listener))
			return false;
	}

	return true;
}

template<typename fn>
void WorldEvents::CallPostFunc(const fn& func)
{
	for (auto listener : m_EventListeners)
		func(listener);
}

void WorldEvents::Init(const std::shared_ptr<WorldState>& world)
{
	if (m_World.lock())
		throw std::logic_error(std::string(__FUNCTION__) + " already called");

	m_World = world;
}

void WorldEvents::AddEventListener(IWorldEventListener* listener)
{
	m_EventListeners.push_back(listener);
}
void WorldEvents::RemoveEventListener(IWorldEventListener* listener)
{
	m_EventListeners.erase(std::find(m_EventListeners.begin(), m_EventListeners.end(), listener));
}

bool WorldEvents::PreDemoCommand(const std::shared_ptr<IDemoCommand>& command)
{
	return CallPreFunc(std::bind(&IWorldEventListener::PreDemoCommand, std::placeholders::_1, m_World.lock(), command));
}
void WorldEvents::PostDemoCommand(const std::shared_ptr<IDemoCommand>& command)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, &command](IWorldEventListener* listener)
	{
		listener->PostDemoCommand(locked, command);
	});
}
bool WorldEvents::PreNetMessage(const std::shared_ptr<INetMessage>& message)
{
	auto locked = m_World.lock();
	return CallPreFunc([&locked, &message](IWorldEventListener* listener)
	{
		return listener->PreNetMessage(locked, message);
	});
}
void WorldEvents::PostNetMessage(const std::shared_ptr<INetMessage>& message)
{
	auto locked = m_World.lock();
	return CallPostFunc([&locked, &message](IWorldEventListener* listener)
	{
		listener->PostNetMessage(locked, message);
	});
}

void WorldEvents::EntityCreated(const std::shared_ptr<Entity>& entity)
{
	CallPostFunc([&entity](IWorldEventListener* listener)
	{
		listener->EntityCreated(entity);
	});
}
void WorldEvents::EntityDeleted(const std::shared_ptr<Entity>& entity)
{
	CallPostFunc([&entity](IWorldEventListener* listener)
	{
		listener->EntityDeleted(entity);
	});
}
void WorldEvents::EntityUpdated(const std::shared_ptr<Entity>& entity)
{
	CallPostFunc([&entity](IWorldEventListener* listener)
	{
		listener->EntityUpdated(entity);
	});
}
void WorldEvents::EntityEnteredPVS(const std::shared_ptr<Entity>& entity)
{
	CallPostFunc([&entity](IWorldEventListener* listener)
	{
		listener->EntityEnteredPVS(entity);
	});
}
void WorldEvents::EntityLeftPVS(const std::shared_ptr<Entity>& entity)
{
	CallPostFunc([&entity](IWorldEventListener* listener)
	{
		listener->EntityLeftPVS(entity);
	});
}

void WorldEvents::TempEntityCreated(const std::shared_ptr<TempEntity>& entity)
{
	CallPostFunc([&entity](IWorldEventListener* listener)
	{
		listener->TempEntityCreated(entity);
	});
}

void WorldEvents::DemoTickChanged(int delta)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, delta](IWorldEventListener* listener)
	{
		listener->DemoTickChanged(locked, delta);
	});
}
void WorldEvents::TickChanged(uint_fast32_t delta)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, delta](IWorldEventListener* listener)
	{
		listener->TickChanged(locked, delta);
	});
}

void WorldEvents::PlayerAdded(const std::shared_ptr<Player>& player)
{
	CallPostFunc(std::bind(&IWorldEventListener::PlayerAdded, std::placeholders::_1, player));
}
void WorldEvents::PlayerRemoved(const std::shared_ptr<Player>& player)
{
	CallPostFunc(std::bind(&IWorldEventListener::PlayerRemoved, std::placeholders::_1, player));
}

void WorldEvents::GameEventFired(const std::shared_ptr<GameEvent>& event)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, &event](IWorldEventListener* listener)
	{
		listener->GameEventFired(locked, event);
	});
}

bool WorldEvents::PreViewEntityUpdate(bool& shouldUpdate, uint_fast16_t& viewEntity)
{
	auto locked = m_World.lock();
	return CallPreFunc([&locked, &shouldUpdate, &viewEntity](IWorldEventListener* listener)
	{
		return listener->PreViewEntityUpdate(locked, shouldUpdate, viewEntity);
	});
}
void WorldEvents::PostViewEntityUpdate()
{
	auto locked = m_World.lock();
	CallPostFunc([&locked](IWorldEventListener* listener)
	{
		listener->PostViewEntityUpdate(locked);
	});
}

bool WorldEvents::PreSignonStateUpdate(SignonState& newState)
{
	auto locked = m_World.lock();
	return CallPreFunc([&locked, &newState](IWorldEventListener* listener)
	{
		return listener->PreSignonStateUpdate(locked, newState);
	});
}
void WorldEvents::PostSignonStateUpdate()
{
	auto locked = m_World.lock();
	return CallPostFunc([&locked](IWorldEventListener* listener)
	{
		listener->PostSignonStateUpdate(locked);
	});
}

void WorldEvents::ServerTextMessage(const std::string_view& msg)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, &msg](IWorldEventListener* listener)
	{
		return listener->ServerTextMessage(locked, msg);
	});
}
void WorldEvents::ServerConVarSet(const std::string_view& name, const std::string_view& value)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, &name, &value](IWorldEventListener* listener)
	{
		listener->ServerConVarSet(locked, name, value);
	});
}
void WorldEvents::ServerConCommand(const std::string_view& cmd, const std::string_view& args)
{
	auto locked = m_World.lock();
	CallPostFunc([&locked, &cmd, &args](IWorldEventListener* listener)
	{
		listener->ServerConCommand(locked, cmd, args);
	});
}

void WorldEvents::PostStringTableCreate(const std::shared_ptr<StringTable>& table)
{
	CallPostFunc([&table](IWorldEventListener* listener)
	{
		listener->PostStringTableCreate(table);
	});
}
void WorldEvents::PostStringTableUpdate(const std::shared_ptr<StringTable>& table)
{
	CallPostFunc([&table](IWorldEventListener* listener)
	{
		listener->PostStringTableUpdate(table);
	});
}

void WorldEvents::PostSendTableListLoad()
{
	CallPostFunc(std::bind(&IWorldEventListener::PostSendTableListLoad, std::placeholders::_1, m_World.lock()));
}
bool WorldEvents::PreGameEventListLoad(std::vector<GameEventDeclaration>& eventList)
{
	auto locked = m_World.lock();
	return CallPreFunc([&locked, &eventList](IWorldEventListener* listener)
	{
		return listener->PreGameEventListLoad(locked, eventList);
	});
}
void WorldEvents::PostGameEventListLoad()
{
	CallPostFunc(std::bind(&IWorldEventListener::PostGameEventListLoad, std::placeholders::_1, m_World.lock()));
}
bool WorldEvents::PreServerClassListLoad(std::vector<std::shared_ptr<const ServerClass>>& classList)
{
	auto locked = m_World.lock();
	auto testLambda = [&locked, &classList](IWorldEventListener* listener)
	{
		return listener->PreServerClassListLoad(locked, classList);
	};

	static constexpr auto test = sizeof(testLambda);

	return CallPreFunc([&locked, &classList](IWorldEventListener* listener)
	{
		return listener->PreServerClassListLoad(locked, classList);
	});
}
void WorldEvents::PostServerClassListLoad()
{
	CallPostFunc(std::bind(&IWorldEventListener::PostServerClassListLoad, std::placeholders::_1, m_World.lock()));
}
void WorldEvents::PostServerInfoLoad()
{
	CallPostFunc(std::bind(&IWorldEventListener::PostServerInfoLoad, std::placeholders::_1, m_World.lock()));
}
bool WorldEvents::PreVoiceSetupLoad(VoiceSetup& setup)
{
	auto locked = m_World.lock();
	return CallPreFunc([&locked, &setup](IWorldEventListener* listener)
	{
		return listener->PreVoiceSetupLoad(locked, setup);
	});
}
void WorldEvents::PostVoiceSetupLoad()
{
	auto locked = m_World.lock();
	CallPostFunc([&locked](IWorldEventListener* listener)
	{
		listener->PostVoiceSetupLoad(locked);
	});
}