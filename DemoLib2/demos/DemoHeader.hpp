#pragma once

#include "BitIO/IStreamElement.hpp"
#include "net/data/SourceConstants.hpp"

#include <cstdint>

class DemoHeader : public IStreamElement
{
public:
	const auto GetMagicToken() const { return m_MagicToken; }
	auto GetDemoProtocol() const { return m_DemoProtocol; }
	auto GetNetworkProtocol() const { return m_NetworkProtocol; }

	const auto GetServerName() const { return m_ServerName; }
	const auto GetClientName() const { return m_ClientName; }
	const auto GetMapName() const { return m_MapName; }
	const auto GetGameDirectory() const { return m_GameDirectory; }

	auto GetPlaybackTime() const { return m_PlaybackTime; }
	auto GetPlaybackTicks() const { return m_PlaybackTicks; }
	auto GetPlaybackFrames() const { return m_PlaybackFrames; }
	auto GetSignonLength() const { return m_SignonLength; }

	static constexpr uint_fast16_t DEMO_HEADER_SIZE = 1072;

protected:
	void ReadElementInternal(BitIOReader& reader) override;
	void WriteElementInternal(BitIOWriter& writer) const override;
	IStreamElement* CreateNewInstance() const override { return new DemoHeader(); }

private:
	char m_MagicToken[8];
	uint32_t m_DemoProtocol;
	uint32_t m_NetworkProtocol;

	char m_ServerName[MAX_OSPATH];
	char m_ClientName[MAX_OSPATH];
	char m_MapName[MAX_OSPATH];
	char m_GameDirectory[MAX_OSPATH];

	float m_PlaybackTime;
	uint32_t m_PlaybackTicks;
	uint32_t m_PlaybackFrames;
	uint32_t m_SignonLength;
};