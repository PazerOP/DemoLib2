#include "EntityCoder.hpp"

#include "BitIO/BitIOReader.hpp"
#include "net/data/SendTable.hpp"
#include "net/data/SourceConstants.hpp"
#include "net/entities/Entity.hpp"
#include "net/entities/IBaseEntity.hpp"
#include "net/worldstate/WorldState.hpp"

#pragma warning(push)
#pragma warning(disable : 4715)	// Not all control paths return a value
uint_fast32_t EntityCoder::ReadUBitVar(BitIOReader& reader)
{
	const auto temp = reader.ReadInline<uint8_t>(6);
	switch (temp & 0b11)
	{
		case 0: return (temp >> 2);                                         // 4 bits
		case 1: return (temp >> 2) | reader.ReadInline<uint8_t>(4) << 4;    // 8 bits
		case 2: return (temp >> 2) | reader.ReadInline<uint8_t>(8) << 4;    // 12 bits
		case 3: return (temp >> 2) | reader.ReadInline<uint32_t>(28) << 4;  // 32 bits
	}
}
#pragma warning(pop)

uint_fast32_t EntityCoder::ReadFieldIndex(BitIOReader& reader, uint_fast32_t lastIndex)
{
	if (!reader.ReadBit())
		return (uint_fast32_t)-1;

	auto diff = ReadUBitVar(reader);
	return lastIndex + diff + 1;
}

uint_fast32_t EntityCoder::ApplyEntityUpdate(IBaseEntity& e, BitIOReader& reader, uint_fast32_t currentTick)
{
	assert(e.GetNetworkTable()->GetFlattenedProperties().size() == e.GetProperties().size());

	uint_fast32_t propsUpdated = 0;
	uint_fast32_t index = (uint_fast32_t)-1;
	while ((index = ReadFieldIndex(reader, index)) != (uint_fast32_t)-1)
	{
		assert(index < MAX_DATATABLE_PROPS);

		auto& sendProp = e[index];

		sendProp.m_LastChangedTick = currentTick;
		if (sendProp.GetDefinition()->Decode(reader, sendProp.GetDataUnion()))
			sendProp.OnValueChanged(sendProp);

		propsUpdated++;
	}

	if (propsUpdated)
		e.OnPropertiesUpdated(e);

	return propsUpdated;
}