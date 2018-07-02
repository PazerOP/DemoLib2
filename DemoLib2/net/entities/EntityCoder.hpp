#pragma once

#include <cstdint>
#include <memory>

class BitIOReader;
class IBaseEntity;

class EntityCoder
{
public:
	EntityCoder() = delete;

	static uint_fast32_t ReadUBitVar(BitIOReader& reader);
	static uint_fast32_t ReadFieldIndex(BitIOReader& reader, uint_fast32_t lastIndex);

	static uint_fast32_t ApplyEntityUpdate(IBaseEntity& e, BitIOReader& reader, uint_fast32_t currentTick);
};