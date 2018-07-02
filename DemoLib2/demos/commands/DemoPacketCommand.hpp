#pragma once

#include "BitIO/BitIOReader.hpp"
#include "demos/DemoViewpoint.hpp"
#include "demos/commands/TimestampedDemoCommand.hpp"

#include <memory>
#include <optional>
#include <vector>

class INetMessage;

class DemoPacketCommand : public TimestampedDemoCommand
{
public:
	virtual ~DemoPacketCommand() = default;

	DemoCommandType GetType() const override { return DemoCommandType::dem_packet; }

	auto& GetMessages() { return m_Messages; }
	const auto& GetMessages() const { return m_Messages; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoPacketCommand(); }

	void ApplyWorldState(WorldState& world) const override;

private:
	DemoViewpoint m_Viewpoint;

	int32_t m_SequenceIn;
	int32_t m_SequenceOut;

	std::vector<std::shared_ptr<INetMessage>> m_Messages;
};