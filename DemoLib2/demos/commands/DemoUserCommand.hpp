#pragma once

#include "BitIO/BitIOReader.hpp"
#include "demos/commands/TimestampedDemoCommand.hpp"

class DemoUserCommand final : public TimestampedDemoCommand
{
public:
	DemoCommandType GetType() const override { return DemoCommandType::dem_usercmd; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoUserCommand(); }

	void ApplyWorldState(WorldState& world) const override;

private:
	uint32_t m_OutgoingSequence;

	BitIOReader m_Data;
};