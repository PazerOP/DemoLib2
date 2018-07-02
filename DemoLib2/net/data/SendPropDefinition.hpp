#pragma once
#include "BitIO/IStreamElement.hpp"
#include "net/data/SendPropFlags.hpp"
#include "net/data/SendPropType.hpp"
#include "net/data/SendPropVariant.hpp"

#include <memory>
#include <optional>
#include <string>

class BitIOReader;
class SendProp;
union SendPropData;
class SendTable;

class SendPropDefinition final : public IStreamElement
{
public:
	SendPropDefinition(const std::weak_ptr<SendTable>& parent, const std::shared_ptr<SendPropDefinition>& arrayElementProp);

	SendPropType GetType() const { return m_Type; }
	const std::string& GetName() const { return m_Name; }
	const std::string& GetFullName() const { return m_FullName; }
	SendPropFlags GetFlags() const { return m_Flags; }
	const std::string& GetExcludeName() const { return m_ExcludeName.value(); }

	auto& GetArrayProperty() { return m_ArrayProperty; }
	auto GetArrayProperty() const { return std::const_pointer_cast<const SendPropDefinition>(m_ArrayProperty); }
	const auto GetArrayElements() const { return m_ArrayElements; }

	const auto GetBitCount() const { return m_BitCount; }

	// So DemoDataTablesCommand can hook up referenced DataTables after
	// we've finished parsing them all.
	void TryConnectDataTable(const std::shared_ptr<SendTable>& table);

	std::shared_ptr<SendTable> GetTable() const { return m_Table.lock(); }
	std::shared_ptr<SendTable> GetParent() const { return m_Parent.lock(); }

	// Returns true if the SendPropData was changed
	bool Decode(BitIOReader& reader, SendPropData& data) const;

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new SendPropDefinition(m_Parent, nullptr); }

private:
	std::string m_FullName;
	std::string m_Name;
	SendPropType m_Type;
	SendPropFlags m_Flags;

	bool DecodeInt(BitIOReader& reader, int32_t& data) const;
	bool DecodeVector(BitIOReader& reader, Vector& data) const;
	bool DecodeFloat(BitIOReader& reader, float& data) const;
	bool DecodeString(BitIOReader& reader, char*& data) const;
	bool DecodeArray(BitIOReader& reader, SendPropData*& data) const;
	bool DecodeVectorXY(BitIOReader& reader, VectorXY& data) const;

	static float DecodeBitNormal(BitIOReader& reader);

	// If we're of SendPropType::DataTable, this variable is unused. Instead,
	// we use it to temporarily store the name of the referenced DataTable until
	// it can be hooked up later in TryConnectDataTable().
	std::optional<std::string> m_ExcludeName;

	uint_fast16_t m_ArrayElements;
	std::shared_ptr<SendPropDefinition> m_ArrayProperty;

	float m_LowValue;
	float m_HighValue;
	uint_fast8_t m_BitCount;

	// If we're SendPropType::Datatable
	std::weak_ptr<SendTable> m_Table;

	std::weak_ptr<SendTable> m_Parent;
};