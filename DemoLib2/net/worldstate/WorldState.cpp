#include "WorldState.hpp"

#include "net/data/KnownStringTable.hpp"
#include "net/data/StringTable.hpp"
#include "net/data/UserInfo.hpp"
#include "net/entities/Entity.hpp"

std::shared_ptr<WorldState> WorldState::Create()
{
	auto retVal = std::shared_ptr<WorldState>(new WorldState());
	retVal->m_Events.Init(retVal);
	return retVal;
}

bool WorldState::SetSignonState(const SignonState& state)
{
	auto clone = state;
	if ((!m_SignonState.has_value() || state != m_SignonState.value()) &&
		m_Events.PreSignonStateUpdate(clone))
	{
		m_SignonState = clone;
		m_Events.PostSignonStateUpdate();
		return true;
	}

	return false;
}

std::optional<BitIOReader> WorldState::GetStaticBaseline(uint_fast16_t serverClassIndex) const
{
	const auto& staticBaselinesTable = *GetStringTable(KnownStringTable::StaticBaselines);

	const auto& foundBL = std::find_if(staticBaselinesTable.begin(), staticBaselinesTable.end(),
		[serverClassIndex](const auto& bl) { return std::strtoul(bl.GetString().c_str(), nullptr, 0) == serverClassIndex; });

	if (foundBL == staticBaselinesTable.end())
		return std::nullopt;

	BitIOReader retVal = foundBL->GetUserDataReader();
	assert(retVal.GetLocalPosition() == BitPosition::Zero());
	return retVal;
}

std::shared_ptr<StringTable> WorldState::GetStringTable(KnownStringTable table)
{
	return std::const_pointer_cast<StringTable>(std::as_const(*this).GetStringTable(table));
}

std::shared_ptr<const StringTable> WorldState::GetStringTable(KnownStringTable table) const
{
	auto& retVal = m_StringTables[(int)table];

	assert(table != KnownStringTable::Downloadables || retVal->GetName() == "downloadables");
	assert(table != KnownStringTable::ModelPrecache || retVal->GetName() == "modelprecache");
	assert(table != KnownStringTable::GenericPrecache || retVal->GetName() == "genericprecache");
	assert(table != KnownStringTable::SoundPrecache || retVal->GetName() == "soundprecache");
	assert(table != KnownStringTable::DecalPrecache || retVal->GetName() == "decalprecache");
	assert(table != KnownStringTable::Userinfo || retVal->GetName() == "userinfo");
	assert(table != KnownStringTable::Lightstyles || retVal->GetName() == "lightstyles");
	assert(table != KnownStringTable::StaticBaselines || retVal->GetName() == "instancebaseline");

	return retVal;
}

std::shared_ptr<StringTable> WorldState::FindStringTable(const std::string_view& tableName)
{
	for (const auto& table : m_StringTables)
	{
		if (table->GetName() == tableName)
			return table;
	}

	return nullptr;
}

bool WorldState::IsStringTableCreated(KnownStringTable table) const
{
	return m_StringTableCount > (int)table;
}

std::shared_ptr<Entity> WorldState::FindEntity(uint_fast32_t handleValue)
{
	return FindEntity(
		handleValue & BitRange(0, MAX_EDICT_BITS),
		(handleValue >> MAX_EDICT_BITS) & BitRange(0, NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS));
}

std::shared_ptr<Entity> WorldState::FindEntity(uint_fast16_t entindex, uint_fast16_t serialNum)
{
	const auto& potential = m_Entities[entindex];

	return (potential && potential->GetSerialNumber() == serialNum) ? potential : nullptr;
}

bool WorldState::GetUserByIndex(uint_fast8_t index, UserInfo& info) const
{
	auto clone = GetStringTable(KnownStringTable::Userinfo)->Get(index).GetUserDataReader();
	if (!clone.Length())
		return false;

	assert(clone.GetLocalPosition().IsZero());

	info.ReadElement(clone);
	return true;
}


bool WorldState::FindUserByID(uint_fast16_t userID, UserInfo* info, uint_fast8_t* entindex) const
{
	UserInfo temp;
	if (!info)
		info = &temp;

	for (uint_fast8_t i = 0; i < m_ServerInfo->m_MaxClients; i++)
	{
		if (GetUserByIndex(i, *info) && info->m_UserID == userID)
		{
			if (entindex)
				*entindex = i + 1;

			return true;
		}
	}

	return false;
}