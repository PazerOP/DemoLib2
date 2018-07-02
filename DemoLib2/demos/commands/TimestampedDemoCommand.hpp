#pragma once

#include "demos/commands/IDemoCommand.hpp"

class TimestampedDemoCommand : public IDemoCommand
{
public:
	virtual ~TimestampedDemoCommand() = default;

	uint_fast32_t GetTick() const { return m_Tick; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;

	void ApplyWorldState(WorldState& world) const override;

private:
	uint_fast32_t m_Tick;
};