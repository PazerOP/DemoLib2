#pragma once

#include "demos/DemoCommandType.hpp"
#include "demos/DemoHeader.hpp"
#include "net/data/IGameStreamElement.hpp"

#include <memory>
#include <vector>

class IDemoCommand;

class DemoFile final : public IGameStreamElement
{
public:
	auto& GetCommands() { return m_Commands; }
	const auto& GetCommands() const { return m_Commands; }

	void ApplyWorldState(WorldState& world) const override;

	using DemoCmdVec = std::vector<std::shared_ptr<IDemoCommand>>;

	static std::shared_ptr<IDemoCommand> ReadCommand(BitIOReader& reader);
	static DemoCmdVec& ReadCommands(BitIOReader& reader, DemoCmdVec& commands);

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoFile(); }

private:
	static std::shared_ptr<IDemoCommand> CreateDemoCommand(DemoCommandType type);

	DemoHeader m_Header;
	DemoCmdVec m_Commands;
};