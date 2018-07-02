#include "SendProp.hpp"

#include "misc/Exceptions.hpp"
#include "net/data/SendPropDefinition.hpp"
#include "net/data/SendTable.hpp"
#include "net/entities/Entity.hpp"
#include "net/worldstate/WorldState.hpp"

SendProp::SendProp(IBaseEntity* entity, const std::shared_ptr<const SendPropDefinition>& definition) : m_Entity(entity), m_Definition(definition)
{
	m_LastChangedTick = 0;

	memset(&m_Data, 0, sizeof(m_Data));
}

SendProp::~SendProp()
{
	if (m_Definition)
	{
		if (m_Definition->GetType() == SendPropType::Array && m_Data.m_Array)
		{
			if (m_Definition->GetArrayProperty()->GetType() == SendPropType::String)
			{
				const auto arrayElems = m_Definition->GetArrayElements();
				for (size_t i = 0; i < arrayElems; i++)
					delete[] m_Data.m_Array[i].m_String;
			}

			delete[] m_Data.m_Array;
		}
		else if (m_Definition->GetType() == SendPropType::String)
			delete[] m_Data.m_String;
	}
}

void SendProp::Clear()
{
	if (m_Definition)
	{
		if (m_Definition->GetType() == SendPropType::Array && m_Data.m_Array)
		{
			const auto arrayElems = m_Definition->GetArrayElements();
			if (m_Definition->GetArrayProperty()->GetType() == SendPropType::String)
			{
				for (size_t i = 0; i < arrayElems; i++)
				{
					if (m_Data.m_Array[i].m_String)
						m_Data.m_Array[i].m_String[0] = '\0';	// "Clear" the strings
				}
			}
			else
			{
				assert(m_Definition->GetArrayProperty()->GetType() != SendPropType::Array);
				for (size_t i = 0; i < arrayElems; i++)
					memset(&m_Data, 0, sizeof(m_Data));	// Just zero out the memory
			}
		}
		else if (m_Definition->GetType() == SendPropType::String && m_Data.m_String)
			m_Data.m_String[0] = '\0';	// "Clear" the string
	}

	m_LastChangedTick = 0;
}

void SendProp::UpdateLastChangedTick()
{
	m_LastChangedTick = GetEntity()->GetWorld()->m_Tick;
}

void SendProp::SetData(const std::string_view& data)
{
	assert(m_Definition->GetType() == SendPropType::String);
	assert(data.size() <= MAX_DT_STRING_LENGTH);

	if (!m_Data.m_String)
		m_Data.m_String = new char[MAX_DT_STRING_LENGTH];

	strncpy_s(m_Data.m_String, MAX_DT_STRING_LENGTH, data.data(), data.size());
}

std::shared_ptr<Entity> SendProp::GetEHandleAsEntity() const
{
	assert(m_Definition->GetType() == SendPropType::Int);
	assert(m_Definition->GetBitCount() == NUM_NETWORKED_EHANDLE_BITS);
	return m_Entity->GetWorld()->FindEntity((uint_fast32_t)m_Data.m_Int);
}