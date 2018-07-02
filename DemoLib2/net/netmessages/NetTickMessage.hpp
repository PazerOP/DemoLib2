#pragma once

#include "net/netmessages/INetMessage.hpp"

class NetTickMessage : public INetMessage
{
public:
	void GetDescription(std::ostream& description) const override;
	void ApplyWorldState(WorldState& world) const override;
	NetMessageType GetType() const override { return NetMessageType::NET_TICK; }

	uint_fast32_t GetTick() const { return m_Tick; }
	float GetFrameTime() const { return m_HostFrameTime; }
	float GetFrameTimeStdDev() const { return m_HostFrameTime; }

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new NetTickMessage(); }

private:
	static constexpr uint_fast8_t TICK_BITS = 32;
	static constexpr uint_fast8_t FLOAT_BITS = 16;
	static constexpr double NET_TICK_SCALEUP = 100000.0;

	uint32_t m_Tick;
	float m_HostFrameTime;
	float m_HostFrameTimeStdDev;
};