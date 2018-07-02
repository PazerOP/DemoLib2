#pragma once

#include "demos/commands/DemoPacketCommand.hpp"

class DemoSignonCommand final : public DemoPacketCommand
{
public:
	DemoCommandType GetType() const override { return DemoCommandType::dem_signon; }

protected:
	void ReadElementInternal(BitIOReader& reader) override { DemoPacketCommand::ReadElementInternal(reader); }
	void WriteElementInternal(BitIOWriter& writer) const override { DemoPacketCommand::WriteElementInternal(writer); }
	IStreamElement* CreateNewInstance() const override { return new DemoSignonCommand(); }
};