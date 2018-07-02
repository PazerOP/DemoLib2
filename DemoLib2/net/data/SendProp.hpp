#pragma once

#include "misc/Event.hpp"
#include "net/data/SendPropData.hpp"
#include "net/data/SendPropDefinition.hpp"
#include "net/entities/EntityCoder.hpp"

#include <memory>
#include <vector>

class Entity;
class IBaseEntity;
class SendPropDefinition;

class SendPropUpdatable
{
protected:
	friend uint_fast32_t EntityCoder::ApplyEntityUpdate(IBaseEntity& e, BitIOReader& reader, uint_fast32_t currentTick);

	uint_fast32_t m_LastChangedTick;
};

template<typename T> struct SendPropGetReturnType
{
	using type = const T&;
};
template<> struct SendPropGetReturnType<bool>
{
	using type = const bool&;
};
template<> struct SendPropGetReturnType<Entity>
{
	using type = std::shared_ptr<Entity>;
};

// Instantiation of a SendPropDefinition (actually stores the data).
class SendProp final : public SendPropUpdatable
{
public:
	SendProp(IBaseEntity* entity, const std::shared_ptr<const SendPropDefinition>& definition);
	SendProp(const SendProp& prop) = delete;    // Move only
	SendProp(SendProp&& prop) = default;
	~SendProp();

	SendProp& operator=(const SendProp& prop) = delete;
	SendProp& operator=(SendProp&& prop) = default;

	const IBaseEntity* GetEntity() const { return m_Entity; }
	IBaseEntity* GetEntity() { return m_Entity; }

	const auto& GetDefinition() const { return m_Definition; }
	const std::string& GetName() const { return m_Definition->GetName(); }
	const std::string& GetFullName() const { return m_Definition->GetFullName(); }

	auto GetLastChangedTick() const { return m_LastChangedTick; }

	template<typename T> typename SendPropGetReturnType<T>::type Get() const;
	template<> std::shared_ptr<Entity> Get<Entity>() const { return GetEHandleAsEntity(); }
	void SetData(int32_t data);
	void SetData(float data);
	void SetData(const Vector& data);
	void SetData(const VectorXY& data);
	void SetData(const std::string_view& data);
	//template<typename T> std::remove_const_t<typename SendPropGetReturnType<T>::type>& Get()
	//{ return const_cast<std::remove_const_t<typename SendPropGetReturnType<T>::type>&>(std::as_const(*this).Get<T>()); }

	auto& GetDataUnion() { return m_Data; }

	void Clear();

private:
	void UpdateLastChangedTick();
	std::shared_ptr<Entity> GetEHandleAsEntity() const;

	std::shared_ptr<const SendPropDefinition> m_Definition;

	SendPropData m_Data;

	IBaseEntity* m_Entity;

public:
	// Down here so debugging views are better
	Event<SendProp&> OnValueChanged;
};

template<typename T> inline typename SendPropGetReturnType<T>::type SendProp::Get() const
{
	static_assert(std::is_enum_v<T>);

	assert(m_Definition->GetType() == SendPropType::Int);
	return *reinterpret_cast<const T*>(&m_Data.m_Int);
}

template<> __forceinline const int32_t& SendProp::Get<int32_t>() const
{
	assert(m_Definition->GetType() == SendPropType::Int);
	return m_Data.m_Int;
}
template<> __forceinline const uint32_t& SendProp::Get<uint32_t>() const
{
	assert(!!(m_Definition->GetFlags() & SendPropFlags::Unsigned));
	return *reinterpret_cast<const uint32_t*>(&Get<int32_t>());
}
template<> __forceinline const float& SendProp::Get<float>() const
{
	assert(m_Definition->GetType() == SendPropType::Float);
	return m_Data.m_Float;
}
template<> __forceinline const Vector& SendProp::Get<Vector>() const
{
	assert(m_Definition->GetType() == SendPropType::Vector);
	return m_Data.m_Vector;
}
template<> __forceinline const VectorXY& SendProp::Get<VectorXY>() const
{
	assert(m_Definition->GetType() == SendPropType::VectorXY);
	return m_Data.m_VectorXY;
}
template<> __forceinline const bool& SendProp::Get<bool>() const
{
	assert(m_Definition->GetType() == SendPropType::Int);
	assert(m_Definition->GetBitCount() == 1);
	return *reinterpret_cast<const bool*>(&m_Data.m_Int);
}

__forceinline void SendProp::SetData(int32_t data)
{
	assert(m_Definition->GetType() == SendPropType::Int);
	m_Data.m_Int = data;
}
__forceinline void SendProp::SetData(float data)
{
	assert(m_Definition->GetType() == SendPropType::Float);
	m_Data.m_Float = data;
}
__forceinline void SendProp::SetData(const Vector& data)
{
	assert(m_Definition->GetType() == SendPropType::Vector);
	m_Data.m_Vector = data;
}
__forceinline void SendProp::SetData(const VectorXY& data)
{
	assert(m_Definition->GetType() == SendPropType::VectorXY);
	m_Data.m_VectorXY = data;
}