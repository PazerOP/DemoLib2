#pragma once

#include "misc/Event.hpp"
#include "net/data/GameEventDeclaration.hpp"
#include "net/data/SendProp.hpp"
#include "net/data/SendTable.hpp"
#include "net/data/ServerClass.hpp"
#include "net/data/ServerInfo.hpp"
#include "net/data/SignonState.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/data/StringTable.hpp"
#include "net/data/VoiceSetup.hpp"
#include "net/worldstate/WorldEvents.hpp"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Entity;
class IWorldEventListener;
class UserInfo;

enum class KnownStringTable : uint_fast8_t;

class WorldState : public std::enable_shared_from_this<WorldState>
{
	WorldState() = default;
public:
	static std::shared_ptr<WorldState> Create();
	WorldState(const WorldState& other) = delete;
	WorldState(WorldState&& other) = delete;
	WorldState& operator=(const WorldState& other) = delete;
	WorldState& operator=(WorldState&& other) = delete;

	auto GetShared() { return shared_from_this(); }
	auto GetShared() const { return shared_from_this(); }
	auto GetWeak() { return weak_from_this(); }
	auto GetWeak() const { return weak_from_this(); }

	uint_fast8_t GetClassBits() const { return Log2Ceil(m_ServerClasses.size()); }

	std::shared_ptr<StringTable> GetStringTable(KnownStringTable table);
	std::shared_ptr<const StringTable> GetStringTable(KnownStringTable table) const;
	std::shared_ptr<StringTable> FindStringTable(const std::string_view& tableName);
	bool IsStringTableCreated(KnownStringTable table) const;

	bool GetUserByEntindex(uint_fast8_t entindex, UserInfo& info) const { return GetUserByIndex(entindex - 1, info); }
	bool GetUserByIndex(uint_fast8_t index, UserInfo& info) const;
	bool FindUserByID(uint_fast16_t userID, UserInfo* info, uint_fast8_t* entindex = nullptr) const;

	std::shared_ptr<Entity> FindEntity(uint_fast16_t entindex, uint_fast16_t serialNum);
	std::shared_ptr<Entity> FindEntity(uint_fast32_t handleValue);

	bool SetSignonState(const SignonState& newState);

	std::optional<BitIOReader> GetStaticBaseline(uint_fast16_t serverClassIndex) const;

	WorldEvents m_Events;

	std::optional<uint_fast32_t> m_EndTick;
	uint_fast32_t m_BaseTick = 0;
	uint_fast32_t m_Tick = 0;
	uint_fast32_t m_DemoTick = 0;
	float m_ServerFrameTime = NAN;
	float m_ServerFrameTimeStdDev = NAN;

	std::optional<ServerInfo> m_ServerInfo;

	std::stringstream m_ServerText;

	std::optional<SignonState> m_SignonState;

	std::optional<VoiceSetup> m_VoiceSetup;

	std::optional<uint_fast16_t> m_ViewEntity;
	Vector m_DemoViewPos = vec3_nan;

	std::shared_ptr<Entity> m_Entities[MAX_EDICTS];

	std::vector<std::shared_ptr<const ServerClass>> m_ServerClasses;
	std::map<std::string_view, std::shared_ptr<const SendTable>> m_SendTables;
	std::vector<GameEventDeclaration> m_GameEventDeclarations;

	std::shared_ptr<StringTable> m_StringTables[MAX_STRINGTABLES];
	uint_fast8_t m_StringTableCount = 0;

	std::map<std::string, std::string> m_ConVars;
	bool m_Paused = false;

	std::optional<BitIOReader> m_InstanceBaselines[2][MAX_EDICTS];
};