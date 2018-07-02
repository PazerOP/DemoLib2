#pragma once

#include "net/data/GameEventDeclaration.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>

class GameEvent final
{
public:
	GameEvent(const GameEventDeclaration& declaration);

	const GameEventDeclaration& GetDeclaration() const { return m_Declaration; }

	bool Is(const std::string_view& name) const { return GetDeclaration().m_Name == name; }

	bool GetBool(const std::string_view& keyName, bool defaultValue = false) const;
	int GetInt(const std::string_view& keyName, int defaultValue = 0) const;
	float GetFloat(const std::string_view& keyName, float defaultValue = 0) const;
	std::string GetString(const std::string_view& keyName, const std::string_view& defaultValue = "") const;

	void Set(const std::string_view& keyName, bool value) { m_Values[std::string(keyName)] = value; }
	void Set(const std::string_view& keyName, int value) { m_Values[std::string(keyName)] = value; }
	void Set(const std::string_view& keyName, float value) { m_Values[std::string(keyName)] = value; }
	void Set(const std::string_view& keyName, const std::string_view& value) { m_Values[std::string(keyName)] = std::string(value); }

	auto& GetValues() { return m_Values; }
	const auto& GetValues() const { return m_Values; }

private:
	const GameEventDeclaration& m_Declaration;
	std::map<std::string, std::variant<bool, int, float, std::string>> m_Values;
};