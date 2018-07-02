#pragma once

#include "net/data/IGameStreamElement.hpp"
#include "net/netmessages/NetMessageType.hpp"

class INetMessage : public IGameStreamElement
{
public:
	virtual ~INetMessage() = default;

	virtual NetMessageType GetType() const = 0;

	std::string GetDescription() const;
	virtual void GetDescription(std::ostream& description) const = 0;
};

inline std::ostream& operator<<(std::ostream& str, const INetMessage& msg)
{
	msg.GetDescription(str);
	return str;
}