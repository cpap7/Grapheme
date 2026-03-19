#pragma once
#include "../Common.h"

#include <cstdint>
#include <string>
#include <functional>

namespace Grapheme {

	enum class EDeviceType : uint8_t {
		Capture		= 1,	// Mic input capture
		Loopback	= 2,	// Get audio from desktop/speaker output

		Default = Capture
	};

	enum class EShareMode : uint8_t {
		Shared = 0,
		Exclusive
	};


	struct SAudioDeviceInfo {
		std::string m_name		= "";
		std::string m_device_id = "";			// Platform-specific ID
		bool m_is_default		= false;
	};

	struct SAudioCaptureConfig {
		std::string m_device_id		= "";					// Empty = default
		uint32_t m_sample_rate		= 16000;				// 16kHz is what's expected for the whisper.cpp STT wrapper
		uint32_t m_channels			= 1;					// Mono audio for STT
		uint32_t m_buffer_size_ms	= 5000;					// Ring buffer duration
		EDeviceType m_device_type = EDeviceType::Default;	// Default = mic input capture
	};

	// Callback function signature, which takes in PCM f32 samples + count
	using AudioDataCallbackFn = std::function<void(const float* a_samples, uint32_t a_sample_count)>;
}