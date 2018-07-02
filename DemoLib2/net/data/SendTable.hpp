#pragma once
#include "BitIO/IStreamElement.hpp"
#include "net/data/SendPropDefinition.hpp"

#include <memory>
#include <string>
#include <vector>

class BitIOReader;

class SendTable final : public std::enable_shared_from_this<SendTable>, public IStreamElement
{
public:
	const auto& GetName() const { return m_Name; }
	const auto& GetProperties() const { return m_Properties; }
	const auto& GetFlattenedProperties() const { return m_FlattenedProps; }

	std::shared_ptr<const SendTable> GetBaseTable() const;

	void InitPostDTConnect();

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new SendTable(); }

private:
	bool _unknown0;

	std::string m_Name;

	using SendPropDefVec = std::vector<std::shared_ptr<const SendPropDefinition>>;

	SendPropDefVec m_Properties;

	void BuildExcludes(SendPropDefVec& list) const;
	SendPropDefVec m_Excludes;

	void BuildFlattenedProps();
	void BuildHierarchy(const SendPropDefVec& excludes, SendPropDefVec& allProperties) const;
	void BuildHierarchy_IterateProps(const SendPropDefVec& excludes, SendPropDefVec& localProperties, SendPropDefVec& childDTProperties) const;
	void SortByPriority(SendPropDefVec& props) const;

	SendPropDefVec m_FlattenedProps;
};