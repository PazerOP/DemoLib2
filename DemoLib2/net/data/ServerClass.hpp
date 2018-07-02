#pragma once

#include <memory>
#include <string>

class BitIOReader;
class SendTable;

class ServerClass final
{
public:
	ServerClass(uint16_t id, std::string&& className, const std::shared_ptr<const SendTable>& sendTable);

	auto GetID() const { return m_ID; }
	const auto& GetClassname() const { return m_Classname; }
	const auto& GetSendTable() const { return m_SendTable; }

private:
	uint16_t m_ID;
	std::string m_Classname;
	std::shared_ptr<const SendTable> m_SendTable;
};