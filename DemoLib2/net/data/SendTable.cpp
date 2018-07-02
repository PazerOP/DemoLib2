#include "SendTable.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "net/data/SourceConstants.hpp"

#include <cassert>
#include <string_view>

using namespace std::string_view_literals;

std::shared_ptr<const SendTable> SendTable::GetBaseTable() const
{
	if (m_Properties.empty())
		return nullptr;

	if (auto first = m_Properties[0]; first->GetName() == "baseclass"sv)
		return first->GetTable();

	return nullptr;
}

void SendTable::InitPostDTConnect()
{
	// Ugh, these need to be after we hook up datatable references in DemoDataTablesCommand
	BuildExcludes(m_Excludes);
	m_Excludes.shrink_to_fit();

	BuildFlattenedProps();
}

void SendTable::ReadElementInternal(BitIOReader& reader)
{
	if (GetBaseCmdArgs().m_PrintDemo && (GetBaseCmdArgs().m_PrintVars || GetBaseCmdArgs().m_PrintRaw))
		cc::out << STR_FILEBITS(reader) << cc::fg::yellow << cc::bold << "Reading SendTable..." << cc::endl;

	_unknown0 = reader.ReadBit("_unknown0");

	m_Name = reader.ReadString("m_Name");

	const auto propertyCount = reader.ReadInline<uint_fast32_t>("propertyCount", PROPINFOBITS_NUMPROPS);

	m_Properties.reserve(propertyCount);

	std::shared_ptr<SendPropDefinition> arrayElementProp;
	for (uint_fast32_t i = 0; i < propertyCount; i++)
	{
		auto propDef = std::make_shared<SendPropDefinition>(weak_from_this(), arrayElementProp);
		propDef->ReadElement(reader);

		if (arrayElementProp)
		{
			assert(propDef->GetType() == SendPropType::Array);
			assert(propDef->GetArrayProperty() == arrayElementProp);
			arrayElementProp.reset();
		}

		if (!!(propDef->GetFlags() & SendPropFlags::InsideArray))
		{
			assert(!arrayElementProp);
			assert(!(propDef->GetFlags() & SendPropFlags::ChangesOften));
			arrayElementProp = propDef;
		}
		else
		{
			m_Properties.push_back(propDef);
		}
	}

	if (GetBaseCmdArgs().m_PrintDemo)
		cc::out << STR_FILEBITS(reader) << cc::fg::yellow << "Finished reading SendTable " << m_Name << '.' << cc::endl;
}
void SendTable::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(_unknown0);

	writer.Write(m_Name);

	auto propCount = m_Properties.size();
	for (const auto& prop : m_Properties)
	{
		if (prop->GetType() == SendPropType::Array)
			propCount++;
	}

	writer.Write(propCount, PROPINFOBITS_NUMPROPS);

	for (const auto& prop : m_Properties)
	{
		if (prop->GetType() == SendPropType::Array)
		{
			assert(prop->GetArrayProperty());
			prop->GetArrayProperty()->WriteElement(writer);
		}
		else
			assert(!prop->GetArrayProperty());

		prop->WriteElement(writer);
	}
}

void SendTable::BuildExcludes(SendPropDefVec& excludes) const
{
	for (const auto& prop : m_Properties)
	{
		if (!!(prop->GetFlags() & SendPropFlags::Exclude))
			excludes.push_back(prop);
		else if (prop->GetType() == SendPropType::Datatable)
			prop->GetTable()->BuildExcludes(excludes);
	}
}

void SendTable::BuildFlattenedProps()
{
	m_FlattenedProps.clear();

	BuildHierarchy(m_Excludes, m_FlattenedProps);
	SortByPriority(m_FlattenedProps);

	m_FlattenedProps.shrink_to_fit();
}

void SendTable::BuildHierarchy(const SendPropDefVec& excludes, SendPropDefVec& allProperties) const
{
	std::vector<std::shared_ptr<const SendPropDefinition>> localProperties;

	BuildHierarchy_IterateProps(excludes, localProperties, allProperties);

	for (auto& localProp : localProperties)
		allProperties.emplace_back(std::move(localProp));
}

void SendTable::BuildHierarchy_IterateProps(const SendPropDefVec& excludes, SendPropDefVec& localProperties, SendPropDefVec& childDTProperties) const
{
	for (auto& prop : m_Properties)
	{
		if (!!(prop->GetFlags() & SendPropFlags::Exclude) || std::any_of(excludes.begin(), excludes.end(), [&prop](const auto& prop2) { return prop2 == prop; }))
			continue;

		if (std::any_of(excludes.begin(), excludes.end(),
			[&prop](const auto& e) { return e->GetName() == prop->GetName() && e->GetExcludeName() == prop->GetParent()->GetName(); }))
			continue;

		if (prop->GetType() == SendPropType::Datatable)
		{
			if (!!(prop->GetFlags() & SendPropFlags::Collapsible))
				prop->GetTable()->BuildHierarchy_IterateProps(excludes, localProperties, childDTProperties);
			else
				prop->GetTable()->BuildHierarchy(excludes, childDTProperties);
		}
		else
			localProperties.push_back(prop);
	}
}

void SendTable::SortByPriority(SendPropDefVec& props) const
{
	size_t start = 0;
	for (size_t i = start; i < props.size(); i++)
	{
		if (!!(props[i]->GetFlags() & SendPropFlags::ChangesOften))
		{
			if (i != start)
				std::swap(props[i], props[start]);

			start++;
		}
	}
}
