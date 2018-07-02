#include "SendPropDefinition.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/SendProp.hpp"
#include "net/data/SendTable.hpp"
#include "net/data/SourceConstants.hpp"

#include <cassert>

#define DEBUGMSG(fmt, ...) printf("\t%s\n", strfmt(fmt, __VA_ARGS__).c_str())

SendPropDefinition::SendPropDefinition(const std::weak_ptr<SendTable>& parent,
	const std::shared_ptr<SendPropDefinition>& arrayElementProp)
{
	m_Parent = parent;
	m_ArrayProperty = arrayElementProp;

	m_BitCount = std::numeric_limits<decltype(m_BitCount)>::max();
	m_ArrayElements = std::numeric_limits<decltype(m_ArrayElements)>::max();
	m_LowValue = NAN;
	m_HighValue = NAN;
}

void SendPropDefinition::TryConnectDataTable(const std::shared_ptr<SendTable>& table)
{
	assert(m_Type == SendPropType::Datatable);
	assert((m_Table.expired() && m_ExcludeName.has_value()) || (!m_Table.expired() && !m_ExcludeName.has_value()));

	if (!strcmp(table->GetName().c_str(), GetExcludeName().c_str()))
	{
		assert(m_Table.expired());
		m_Table = table;
	}
}

bool SendPropDefinition::Decode(BitIOReader& reader, SendPropData& data) const
{
	switch (m_Type)
	{
		case SendPropType::Int:      return DecodeInt(reader, data.m_Int);
		case SendPropType::Float:    return DecodeFloat(reader, data.m_Float);
		case SendPropType::Vector:   return DecodeVector(reader, data.m_Vector);
		case SendPropType::VectorXY: return DecodeVectorXY(reader, data.m_VectorXY);
		case SendPropType::String:   return DecodeString(reader, data.m_String);
		case SendPropType::Array:    return DecodeArray(reader, data.m_Array);

		default:
			throw std::runtime_error("Unexpected SendPropType during decoding");
	}
}

void SendPropDefinition::ReadElementInternal(BitIOReader& reader)
{
	reader.Read("SendPropDefinition::m_Type", m_Type, PROPINFOBITS_TYPE);

	assert(m_Type < SendPropType::COUNT);

	m_Name = reader.ReadString("SendPropDefinition::m_Name");
	m_FullName = m_Parent.lock()->GetName() + '.' + m_Name;

	reader.Read("SendPropDefinition::m_Flags", m_Flags, PROPINFOBITS_FLAGS);

	//DEBUGMSG("\tSendPropDefinition: {0} ({1}) with flags {2}", m_Name, EnumToString(m_Type), EnumToString(m_Flags));

	// DataTables have no flags
	//assert(m_Flags == SendPropFlags(0) || m_Type != SendPropType::Datatable);

	if (m_Type == SendPropType::Datatable)
	{
		m_ExcludeName = reader.ReadString("SendPropDefinition::m_ExcludeName (DataTable)");
	}
	else
	{
		if (!!(m_Flags & SendPropFlags::Exclude))
			m_ExcludeName = reader.ReadString("SendPropDefinition::m_ExcludeName");
		else if (m_Type == SendPropType::Array)
			reader.Read("SendPropDefinition::m_ArrayElements", m_ArrayElements, PROPINFOBITS_NUMELEMENTS);
		else
		{
			reader.Read("SendPropDefinition::m_LowValue", m_LowValue);
			reader.Read("SendPropDefinition::m_HighValue", m_HighValue);
			reader.Read("SendPropDefinition::m_BitCount", m_BitCount, PROPINFOBITS_NUMBITS);
		}
	}

	if (!!(m_Flags & SendPropFlags::NoScale))
	{
		if (m_Type == SendPropType::Float)
			m_BitCount = 32;
		else if (m_Type == SendPropType::Vector)
		{
			if (!!(m_Flags & SendPropFlags::Normal))
				m_BitCount = 32 * 3;
		}
	}
}

void SendPropDefinition::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_Type, PROPINFOBITS_TYPE);

	writer.Write(m_Name);
	writer.Write(m_Flags, PROPINFOBITS_FLAGS);

	//if (m_ArrayProperty)
	//  DEBUG_BREAK();

	if (m_Type == SendPropType::Datatable)
	{
		const auto& locked = m_Table.lock();
		if (!locked)
		{
			writer.Write(m_ExcludeName.value());
		}
		else
		{
			const auto& name = locked->GetName();
			writer.Write(name);
		}
	}
	else
	{
		if (!!(m_Flags & SendPropFlags::Exclude))
			writer.Write(m_ExcludeName.value());
		else if (m_Type == SendPropType::Array)
			writer.Write(m_ArrayElements, PROPINFOBITS_NUMELEMENTS);
		else
		{
			uint_fast8_t bitCount = m_BitCount;
			if (!!(m_Flags & SendPropFlags::NoScale))
			{
				if (m_Type == SendPropType::Float)
					bitCount = 0;
				else if (m_Type == SendPropType::Vector)
				{
					if (!!(m_Flags & SendPropFlags::Normal))
						bitCount = 0;
				}
			}

			writer.Write(m_LowValue);
			writer.Write(m_HighValue);
			writer.Write(bitCount, PROPINFOBITS_NUMBITS);
		}
	}
}

#pragma warning(push)
#pragma warning(disable : 4063) // case 'identifier' is not a valid value for switch of enum 'identifier'
bool SendPropDefinition::DecodeInt(BitIOReader& reader, int32_t& data) const
{
	const auto old = data;
	switch (m_Flags & (SendPropFlags::VarInt | SendPropFlags::Unsigned))
	{
		case SendPropFlags::Unsigned:
			reader.Read<uint32_t>(*reinterpret_cast<uint32_t*>(&data), m_BitCount);
			break;
		case (SendPropFlags)0:
			reader.Read<int32_t>(data, m_BitCount);
			break;
		case SendPropFlags::VarInt:
			data = reader.ReadVarInt();
			break;
		case SendPropFlags::VarInt | SendPropFlags::Unsigned:
			data = reader.ReadUVarInt();
			break;

#ifdef _DEBUG
		default:
			assert(!"Unknown combination of SendPropFlags for int");
#endif
	}

	return old != data;
}
#pragma warning(pop)

bool SendPropDefinition::DecodeVector(BitIOReader& reader, Vector& data) const
{
	const auto old = data;

	DecodeFloat(reader, data.x);
	DecodeFloat(reader, data.y);

	if (!(m_Flags & SendPropFlags::Normal))
		DecodeFloat(reader, data.z);
	else
	{
		// Third component gets determined from the first 2
		data.z = std::sqrt(1 - (data.x * data.x + data.y * data.y));

		if (reader.ReadBit())
			data.z = -data.z;
	}

	return old != data;
}

template<bool isLowPrecision> static float DecodeBitCoord(BitIOReader& reader)
{
	float value = 0;

	auto firstBits = reader.ReadInline<uint_fast8_t>(3);

	bool inBounds = firstBits & (1 << 0);
	bool hasIntVal = firstBits & (1 << 1);
	bool isNegative = firstBits & (1 << 2);

	if (hasIntVal)
	{
		if (inBounds)
			value = reader.ReadInline<uint16_t>(COORD_INTEGER_BITS_MP) + 1;
		else
		{
			value = reader.ReadInline<uint16_t>(COORD_INTEGER_BITS) + 1;

			if (value < (1 << COORD_INTEGER_BITS_MP))
				throw new std::runtime_error("Something's fishy...");
		}
	}

	auto fractVal = reader.ReadInline<uint8_t>(isLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);

	value = value + fractVal * (isLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION);

	if (isNegative)
		value = -value;

	return value;
}

static float DecodeBitCoordInt(BitIOReader& reader)
{
	float value = 0;

	auto firstBits = reader.ReadInline<uint_fast8_t>(2);
	bool inBounds = firstBits & (1 << 0);
	bool hasIntVal = firstBits & (1 << 1);

	if (hasIntVal)
	{
		bool isNegative = reader.ReadBit();

		if (inBounds)
			value = reader.ReadInline<uint16_t>(COORD_INTEGER_BITS_MP) + 1;
		else
		{
			value = reader.ReadInline<uint16_t>(COORD_INTEGER_BITS) + 1;

			if (value < (1 << COORD_INTEGER_BITS_MP))
				throw std::runtime_error("Something's fishy...");
		}

		if (isNegative)
			value = -value;
	}

	return value;
}

bool SendPropDefinition::DecodeFloat(BitIOReader& reader, float& data) const
{
	const auto old = data;

	if (!!(m_Flags & SendPropFlags::CoordMPLowPrecision))
		data = DecodeBitCoord<true>(reader);
	else if (!!(m_Flags & SendPropFlags::CoordMP))
		data = DecodeBitCoord<false>(reader);
	else if (!!(m_Flags & SendPropFlags::CoordMPIntegral))
		data = DecodeBitCoordInt(reader);
	else if (!!(m_Flags & SendPropFlags::NoScale))
		data = reader.ReadInline<float>();
	else if (!!(m_Flags & SendPropFlags::Normal))
		data = DecodeBitNormal(reader);
	else if (!!(m_Flags & SendPropFlags::Coord))
		throw NotImplementedException("SendPropFlags::Coord");
	else
	{
		assert(m_BitCount <= 32);
		auto raw = reader.ReadInline<uint32_t>(m_BitCount);
		float percentage = (double)raw / ((1UL << m_BitCount) - 1);
		data = m_LowValue + (m_HighValue - m_LowValue) * percentage;
	}

	return old != data;
}

bool SendPropDefinition::DecodeString(BitIOReader& reader, char*& data) const
{
	bool changed = false;
	char old[MAX_DT_STRING_LENGTH + 1];

	const auto chars = reader.ReadInline<uint16_t>(DT_STRING_BITS);

	if (!data)
	{
		data = new char[MAX_DT_STRING_LENGTH + 1];
		changed = true;
	}
	else
	{
		memcpy(old, data, sizeof(old));
	}

	reader.ReadArray(data, chars);
	data[chars] = '\0';

	return changed || !!memcmp(old, data, sizeof(old));
}

bool SendPropDefinition::DecodeArray(BitIOReader& reader, SendPropData*& data) const
{
	uint_fast8_t numBits = Log2Ceil(m_ArrayElements);

	const auto elementCount = reader.ReadInline<uint16_t>(numBits);

	bool changed = false;
	if (!data)
	{
		data = new SendPropData[m_ArrayElements];
		memset(data, 0, sizeof(*data) * m_ArrayElements);
		changed = true;
	}

	for (uint_fast16_t i = 0; i < elementCount; i++)
		changed = m_ArrayProperty->Decode(reader, data[i]) || changed;

	return changed;
}

bool SendPropDefinition::DecodeVectorXY(BitIOReader& reader, VectorXY& data) const
{
	bool changed = DecodeFloat(reader, data.x);
	changed = DecodeFloat(reader, data.y) || changed;

	return changed;
}

float SendPropDefinition::DecodeBitNormal(BitIOReader& reader)
{
	bool signBit = reader.ReadBit();

	float fractVal = reader.ReadInline<uint16_t>(NORMAL_FRACTIONAL_BITS) * NORMAL_RESOLUTION;

	return signBit ? -fractVal : fractVal;
}
