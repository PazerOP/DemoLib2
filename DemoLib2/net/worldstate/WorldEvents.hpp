#pragma once

#include "net/worldstate/IWorldEventListener.hpp"

#include <memory>
#include <type_traits>
#include <vector>

class IDemoCommand;

class WorldEvents
{
public:
	void Init(const std::shared_ptr<WorldState>& world);

	void AddEventListener(IWorldEventListener* listener);
	void RemoveEventListener(IWorldEventListener* listener);

	bool PreDemoCommand(const std::shared_ptr<IDemoCommand>& command);
	void PostDemoCommand(const std::shared_ptr<IDemoCommand>& command);
	bool PreNetMessage(const std::shared_ptr<INetMessage>& message);
	void PostNetMessage(const std::shared_ptr<INetMessage>& message);

	void EntityCreated(const std::shared_ptr<Entity>& entity);
	void EntityDeleted(const std::shared_ptr<Entity>& entity);
	void EntityUpdated(const std::shared_ptr<Entity>& entity);
	void EntityEnteredPVS(const std::shared_ptr<Entity>& entity);
	void EntityLeftPVS(const std::shared_ptr<Entity>& entity);

	void TempEntityCreated(const std::shared_ptr<TempEntity>& entity);

	void DemoTickChanged(int delta);
	void TickChanged(uint_fast32_t delta);

	void PlayerAdded(const std::shared_ptr<Player>& player);
	void PlayerRemoved(const std::shared_ptr<Player>& player);

	void GameEventFired(const std::shared_ptr<GameEvent>& event);

	bool PreViewEntityUpdate(bool& shouldUpdate, uint_fast16_t& viewEntity);
	void PostViewEntityUpdate();

	bool PreSignonStateUpdate(SignonState& newState);
	void PostSignonStateUpdate();

	void ServerTextMessage(const std::string_view& msg);
	void ServerConVarSet(const std::string_view& name, const std::string_view& value);
	void ServerConCommand(const std::string_view& cmd, const std::string_view& args);

	void PostStringTableCreate(const std::shared_ptr<StringTable>& table);
	void PostStringTableUpdate(const std::shared_ptr<StringTable>& table);

	void PostSendTableListLoad();
	bool PreGameEventListLoad(std::vector<GameEventDeclaration>& eventList);
	void PostGameEventListLoad();
	bool PreServerClassListLoad(std::vector<std::shared_ptr<const ServerClass>>& classList);
	void PostServerClassListLoad();
	void PostServerInfoLoad();
	bool PreVoiceSetupLoad(VoiceSetup& setup);
	void PostVoiceSetupLoad();

private:
	std::weak_ptr<WorldState> m_World;
	std::vector<IWorldEventListener*> m_EventListeners;

	template<typename fn> bool CallPreFunc(const fn& func);
	template<typename fn> void CallPostFunc(const fn& func);
};