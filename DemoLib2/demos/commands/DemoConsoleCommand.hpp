#pragma once

#include "demos/commands/TimestampedDemoCommand.hpp"

#include <string>

class DemoConsoleCommand final : public TimestampedDemoCommand
{
public:
  IStreamElement* CreateNewInstance() const override { return new DemoConsoleCommand(); }

  DemoCommandType GetType() const override { return DemoCommandType::dem_consolecmd; }

protected:
  void ReadElementInternal(BitIOReader& reader) override;
  void WriteElementInternal(BitIOWriter& writer) const override;

  void ApplyWorldState(WorldState& world) const override;

private:
  std::string m_Command;
};