#pragma once

#include "misc/Event.hpp"
#include "net/data/SendProp.hpp"

#include <memory>

class IReadOnlyPropertySet
{
public:
	virtual ~IReadOnlyPropertySet() = default;

	const auto& GetProperties() const { return m_Properties; }
	const SendProp* FindProperty(const SendPropDefinition& def) const;
	const SendProp* FindProperty(const std::string_view& propName) const;

	Vector FindPropVec3(const std::string_view& propNameBase, Vector defaultVal = Vector(0)) const;

	__forceinline const SendProp& operator[](const SendPropDefinition& def) const { auto p = FindProperty(def); assert(p); return *p; }
	__forceinline const SendProp& operator[](const std::string_view& propName) const { auto p = FindProperty(propName); assert(p); return *p; }
	__forceinline const SendProp& operator[](uint_fast16_t propIndex) const { return m_Properties[propIndex]; }

protected:
	std::vector<SendProp> m_Properties;
};

class IPropertySet : public IReadOnlyPropertySet
{
public:
	virtual ~IPropertySet() = default;

	Event<SendProp&> OnPropertyAdded;
	Event<IPropertySet&> OnPropertiesUpdated;

	using IReadOnlyPropertySet::GetProperties;
	auto& GetProperties() { return m_Properties; }

	using IReadOnlyPropertySet::FindProperty;
	SendProp* FindProperty(const SendPropDefinition& def);
	SendProp* FindProperty(const std::string_view& propName);

	using IReadOnlyPropertySet::operator[];
	__forceinline SendProp& operator[](const SendPropDefinition& def) { auto p = FindProperty(def); assert(p); return *p; }
	__forceinline SendProp& operator[](const std::string_view& propName) { auto p = FindProperty(propName); assert(p); return *p; }
	__forceinline SendProp& operator[](uint_fast16_t propIndex) { return m_Properties[propIndex]; }
};