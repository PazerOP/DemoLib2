#include "net/entities/IBaseEntity.hpp"

class TempEntity final : public IBaseEntity
{
public:
	TempEntity(const std::weak_ptr<WorldState>& world, const std::shared_ptr<const ServerClass>& serverClass, const std::shared_ptr<const SendTable>& networkTable) : IBaseEntity(world, serverClass, networkTable) {}

	bool IsTempEntity() const override { return true; }
};