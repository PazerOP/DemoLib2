#include "NetClassInfoMessage.hpp"

#include "BitIO/BitIOReader.hpp"
#include "BitIO/BitIOWriter.hpp"
#include "misc/Exceptions.hpp"
#include "net/data/ServerClass.hpp"
#include "net/worldstate/WorldState.hpp"

void NetClassInfoMessage::ReadElementInternal(BitIOReader& reader)
{
	reader.Read(m_ServerClassCount, 16);
	m_CreateOnClient = reader.ReadBit();
	if (m_CreateOnClient)
		return;

	uint_fast8_t serverClassBits = Log2(m_ServerClassCount) + 1;

	m_ServerClasses.clear();
	m_ServerClasses.reserve(m_ServerClassCount);

	for (uint_fast16_t i = 0; i < m_ServerClassCount; i++)
	{
		auto& simple = m_ServerClasses.emplace_back();
		simple.m_ID = reader.ReadInline<uint16_t>(serverClassBits);
		simple.m_Classname = reader.ReadString();
		simple.m_DatatableName = reader.ReadString();
	}
}
void NetClassInfoMessage::WriteElementInternal(BitIOWriter& writer) const
{
	writer.Write(m_ServerClassCount, 16);
	writer.Write(m_CreateOnClient);
	if (m_CreateOnClient)
		return;

	const uint_fast8_t serverClassBits = Log2(m_ServerClassCount) + 1;

	for (uint_fast16_t i = 0; i < m_ServerClassCount; i++)
	{
		const auto& serverClass = m_ServerClasses[i];
		writer.Write(serverClass.m_ID, serverClassBits);
		writer.Write(serverClass.m_Classname);
		writer.Write(serverClass.m_DatatableName);
	}
}

void NetClassInfoMessage::GetDescription(std::ostream& description) const
{
	description << "svc_ClassInfo: " << m_ServerClassCount << ", " << m_CreateOnClient;
}
void NetClassInfoMessage::ApplyWorldState(WorldState& world) const
{
	if (!m_ServerClasses.size())
		return;

	assert(!world.m_SendTables.empty());

	std::vector<std::shared_ptr<const ServerClass>> clone;
	clone.reserve(m_ServerClasses.size());
	for (const auto& sc : m_ServerClasses)
		clone.emplace_back(std::make_shared<ServerClass>(sc.m_ID, copy(sc.m_Classname), world.m_SendTables.at(sc.m_DatatableName)));

	world.m_Events.PreServerClassListLoad(clone);
	world.m_ServerClasses = std::move(clone);
	world.m_Events.PostServerClassListLoad();
}