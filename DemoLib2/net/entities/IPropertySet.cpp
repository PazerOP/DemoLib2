#include "IPropertySet.hpp"

#include "net/data/SendPropDefinition.hpp"

#include <algorithm>

const SendProp* IReadOnlyPropertySet::FindProperty(const SendPropDefinition& def) const
{
	for (const auto& prop : m_Properties)
	{
		// FIXME: this really should not use pointer comparison
		if (prop.GetDefinition().get() == &def)
			return &prop;
	}

	return nullptr;
}

const SendProp* IReadOnlyPropertySet::FindProperty(const std::string_view& propName) const
{
	for (const auto& prop : m_Properties)
	{
		if (prop.GetFullName() == propName)
			return &prop;
	}

	return nullptr;
}

SendProp* IPropertySet::FindProperty(const SendPropDefinition& def)
{
	return const_cast<SendProp*>(IReadOnlyPropertySet::FindProperty(def));
}

SendProp* IPropertySet::FindProperty(const std::string_view& propName)
{
	return const_cast<SendProp*>(IReadOnlyPropertySet::FindProperty(propName));
}

Vector IReadOnlyPropertySet::FindPropVec3(const std::string_view& propNameBase, Vector defaultVal) const
{
	uint_fast8_t matchCount = 0;

	for (const auto& prop : m_Properties)
	{
		const auto& fullName = prop.GetFullName();

		const auto extraChars = fullName.size() - propNameBase.size();

		if ((extraChars == 0 || extraChars == 3) && fullName.find(propNameBase) == 0)
		{
			if (extraChars == 0)
			{
				if (prop.GetDefinition()->GetType() == SendPropType::Vector)
					return prop.Get<Vector>();
				else
				{
					// Vector2D
					auto firstTwo = prop.Get<VectorXY>();

					defaultVal.x = firstTwo.x;
					defaultVal.y = firstTwo.y;

					matchCount += 2;
				}
			}
			else
			{
				// Get the digit between the [] brackets
				const auto index = fullName[fullName.size() - 2] - '0';
				defaultVal[index] = prop.Get<float>();

				matchCount++;
			}

			if (matchCount == 3)
				break;
		}
	}

	return defaultVal;
}