#pragma once

#include <cstdint>
#include <string>

class VoiceSetup
{
public:
	std::string m_VoiceCodec;
	uint_fast8_t m_Quality;
	uint_fast16_t m_SampleRate;
};