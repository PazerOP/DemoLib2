#include "ServerClass.hpp"

ServerClass::ServerClass(uint16_t id, std::string&& classname, const std::shared_ptr<const SendTable>& sendTable) :
	m_ID(id), m_Classname(classname), m_SendTable(sendTable)
{

}