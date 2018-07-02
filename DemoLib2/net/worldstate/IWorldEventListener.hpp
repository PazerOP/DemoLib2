#pragma once

#include <memory>
#include <vector>

class Entity;
class GameEvent;
struct GameEventDeclaration;
class IDemoCommand;
class INetMessage;
class Player;
class ServerClass;
struct ServerInfo;
struct SignonState;
class StringTable;
class TempEntity;
class VoiceSetup;
class WorldState;

class IWorldEventListener
{
public:
	virtual ~IWorldEventListener() = default;

	virtual bool PreDemoCommand(const std::shared_ptr<WorldState>& world, const std::shared_ptr<IDemoCommand>& command) { return true; }
	virtual void PostDemoCommand(const std::shared_ptr<WorldState>& world, const std::shared_ptr<IDemoCommand>& command) {}
	virtual bool PreNetMessage(const std::shared_ptr<WorldState>& world, const std::shared_ptr<INetMessage>& message) { return true; }
	virtual void PostNetMessage(const std::shared_ptr<WorldState>& world, const std::shared_ptr<INetMessage>& message) {}

	virtual void EntityCreated(const std::shared_ptr<Entity>& ent) {}
	virtual void EntityDeleted(const std::shared_ptr<Entity>& ent) {}
	virtual void EntityUpdated(const std::shared_ptr<Entity>& ent) {}
	virtual void EntityEnteredPVS(const std::shared_ptr<Entity>& ent) {}
	virtual void EntityLeftPVS(const std::shared_ptr<Entity>& ent) {}

	virtual void TempEntityCreated(const std::shared_ptr<TempEntity>& tempEnt) {}

	virtual void DemoTickChanged(const std::shared_ptr<WorldState>& world, int delta) {}
	virtual void TickChanged(const std::shared_ptr<WorldState>& world, uint_fast32_t delta) {}

	virtual void PlayerAdded(const std::shared_ptr<Player>& playerAdded) {}
	virtual void PlayerRemoved(const std::shared_ptr<Player>& playerRemoved) {}

	virtual void GameEventFired(const std::shared_ptr<WorldState>& world, const std::shared_ptr<GameEvent>& event) {}

	virtual bool PreViewEntityUpdate(const std::shared_ptr<WorldState>& world, bool& shouldUpdate, uint_fast16_t& viewEntity) { return true; }
	virtual void PostViewEntityUpdate(const std::shared_ptr<WorldState>& world) {}

	virtual bool PreSignonStateUpdate(const std::shared_ptr<WorldState>& world, SignonState& newState) { return true; }
	virtual void PostSignonStateUpdate(const std::shared_ptr<WorldState>& world) {}

	virtual void ServerTextMessage(const std::shared_ptr<WorldState>& world, const std::string_view& msg) {}
	virtual void ServerConVarSet(const std::shared_ptr<WorldState>& world, const std::string_view& name, const std::string_view& value) {}
	virtual void ServerConCommand(const std::shared_ptr<WorldState>& world, const std::string_view& cmd, const std::string_view& args) {}

	virtual void PostStringTableCreate(const std::shared_ptr<StringTable>& table) {}
	virtual void PostStringTableUpdate(const std::shared_ptr<StringTable>& table) {}

	// "Loaded" events
	virtual void PostSendTableListLoad(const std::shared_ptr<WorldState>& world) {}
	virtual bool PreGameEventListLoad(const std::shared_ptr<WorldState>& world, std::vector<GameEventDeclaration>& eventList) { return true; }
	virtual void PostGameEventListLoad(const std::shared_ptr<WorldState>& world) {}
	virtual bool PreServerClassListLoad(const std::shared_ptr<WorldState>& world, std::vector<std::shared_ptr<const ServerClass>>& classList) { return true; }
	virtual void PostServerClassListLoad(const std::shared_ptr<WorldState>& world) {}
	virtual void PostServerInfoLoad(const std::shared_ptr<WorldState>& world) {}
	virtual bool PreVoiceSetupLoad(const std::shared_ptr<WorldState>& world, VoiceSetup& setup) { return true; }
	virtual void PostVoiceSetupLoad(const std::shared_ptr<WorldState>& world) {}
};