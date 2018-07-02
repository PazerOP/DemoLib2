#pragma once

#include "demos/commands/TimestampedDemoCommand.hpp"

class BitIOReader;

class DemoSyncTickCommand : public TimestampedDemoCommand
{
public:
	DemoCommandType GetType() const override { return DemoCommandType::dem_synctick; }

protected:
	// This demo command type has no data
	void ReadElementInternal(BitIOReader& reader) override { TimestampedDemoCommand::ReadElementInternal(reader); }
	void WriteElementInternal(BitIOWriter& writer) const override { TimestampedDemoCommand::WriteElementInternal(writer); }
	IStreamElement* CreateNewInstance() const override { return new DemoSyncTickCommand(); }

	void ApplyWorldState(WorldState& world) const override;
};