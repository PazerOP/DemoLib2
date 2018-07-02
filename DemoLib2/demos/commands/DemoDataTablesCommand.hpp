#include "demos/commands/TimestampedDemoCommand.hpp"
#include "net/data/SendTable.hpp"
#include "net/data/ServerClass.hpp"

#include <memory>
#include <vector>

class SendTable;
class ServerClass;

class DemoDataTablesCommand final : public TimestampedDemoCommand
{
public:
	DemoCommandType GetType() const override { return DemoCommandType::dem_datatables; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoDataTablesCommand(); }

	void ApplyWorldState(WorldState& world) const override;

private:
	std::vector<std::shared_ptr<SendTable>> m_SendTables;
	std::vector<std::shared_ptr<ServerClass>> m_ServerClasses;

	std::shared_ptr<const SendTable> FindSendTable(const std::string_view& name) const;
};