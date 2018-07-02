#pragma once

#include <cassert>
#include <functional>
#include <vector>

template<class... Args> class Event final
{
	using CallbackType = std::function<void(Args...)>;

public:
	Event() = default;
	Event(const Event<Args...>& other) = delete;
	Event(Event<Args...>&& other) = default;
	__forceinline explicit Event(CallbackType&& callback)
	{
		RegisterCallback(std::move(callback));
	}

	Event<Args...>& operator=(const Event<Args...>& rhs) = delete;
	Event<Args...>& operator=(Event<Args...>&& rhs) = default;

	__forceinline void FireEvent(Args... args) const
	{
		for (const auto& callback : m_Callbacks)
			callback(args...);
	}
	__forceinline void operator()(Args... args) const
	{
		FireEvent(args...);
	}

	__forceinline Event<Args...>& operator+=(CallbackType&& callback)
	{
		RegisterCallback(std::move(callback));
		return *this;
	}
	__forceinline Event<Args...>& operator-=(const CallbackType& callback)
	{
		UnregisterCallback(callback);
		return *this;
	}

	__forceinline void RegisterCallback(CallbackType&& callback)
	{
		m_Callbacks.emplace_back(std::move(callback));
	}
	void UnregisterCallback(const CallbackType& callback)
	{
		const auto target = callback.template target<void(Args...)>();
		assert(target);

		for (size_t i = 0; i < m_Callbacks.size(); i++)
		{
			auto otherTarget = m_Callbacks[i].template target<void(Args...)>();
			assert(otherTarget);

			if (target == otherTarget)
			{
				m_Callbacks.erase(m_Callbacks.begin() + i);
				return;
			}
		}

		assert(!"Failed to find matching callback!");
	}

private:
	std::vector<CallbackType> m_Callbacks;
};