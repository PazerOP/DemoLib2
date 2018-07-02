#include "GameEvent.hpp"

GameEvent::GameEvent(const GameEventDeclaration& declaration) : m_Declaration(declaration)
{
}

bool GameEvent::GetBool(const std::string_view& keyName, bool defaultValue) const
{
	const auto& found = m_Values.find(std::string(keyName));
	if (found == m_Values.end())
		return defaultValue;

	if (auto value = std::get_if<bool>(&found->second))
		return *value;
	if (auto value = std::get_if<int>(&found->second))
		return *value;
	if (auto value = std::get_if<float>(&found->second))
		return (int)*value;
	if (auto value = std::get_if<std::string>(&found->second))
		return value->compare("0");

	throw std::logic_error("Unexpected type in variant");
}

int GameEvent::GetInt(const std::string_view& keyName, int defaultValue) const
{
	const auto& found = m_Values.find(std::string(keyName));
	if (found == m_Values.end())
		return defaultValue;

	if (auto value = std::get_if<bool>(&found->second))
		return *value ? 1 : 0;
	if (auto value = std::get_if<int>(&found->second))
		return *value;
	if (auto value = std::get_if<float>(&found->second))
		return (int)*value;
	if (auto value = std::get_if<std::string>(&found->second))
		return atoi(value->c_str());

	throw std::logic_error("Unexpected type in variant");
}

float GameEvent::GetFloat(const std::string_view & keyName, float defaultValue) const
{
	const auto& found = m_Values.find(std::string(keyName));
	if (found == m_Values.end())
		return defaultValue;

	if (auto value = std::get_if<bool>(&found->second))
		return *value ? 1 : 0;
	if (auto value = std::get_if<int>(&found->second))
		return *value;
	if (auto value = std::get_if<float>(&found->second))
		return *value;
	if (auto value = std::get_if<std::string>(&found->second))
		return std::atof(value->c_str());

	throw std::logic_error("Unexpected type in variant");
}

std::string GameEvent::GetString(const std::string_view& keyName, const std::string_view& defaultValue) const
{
	using namespace std::string_literals;

	const auto& found = m_Values.find(std::string(keyName));
	if (found == m_Values.end())
		return std::string(defaultValue);

	if (auto value = std::get_if<bool>(&found->second))
		return *value ? "1"s : "0"s;
	if (auto value = std::get_if<int>(&found->second))
	{
		char buf[64];
		sprintf_s(buf, "%i", *value);
		return buf;
	}
	if (auto value = std::get_if<float>(&found->second))
	{
		char buf[128];
		sprintf_s(buf, "%f", *value);
		return buf;
	}
	if (auto value = std::get_if<std::string>(&found->second))
		return *value;

	throw std::logic_error("Unexpected type in variant");
}