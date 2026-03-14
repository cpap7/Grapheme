#pragma once
#include "AudioConfig.h"
#include "AudioRingBuffer.h"
#include "../Core/ResultCodes.h"
#include <memory>
#include <atomic>

// Forward declarations
struct ma_device;
struct ma_context;

namespace Grapheme {

	class CAudioDeviceImpl {
	private:
		AudioDataCallbackFn m_user_callback; // Callback from consumer thread to drain buffered audio

		SAudioCaptureConfig m_config;
		CAudioRingBuffer m_ring_buffer;

		std::vector<float> m_mono_buffer; // For downmixing stereo --> mono

		std::unique_ptr<ma_device> m_device = nullptr;
		std::unique_ptr<ma_context> m_context = nullptr;

		std::atomic<bool> m_is_capturing{ false }; // Internal-state

	public:
		explicit CAudioDeviceImpl(const SAudioCaptureConfig& a_config);
		~CAudioDeviceImpl();

	private:
		void Init();

	public:
		//void Reinit();
		void Shutdown();
		EResultCode StartCapture();
		EResultCode StopCapture();

		// Getters
		inline bool IsCapturing() const { return m_is_capturing.load(std::memory_order_acquire); }
		inline bool IsReady() const		{ return m_device != nullptr;							 }
		
		// Setters
		inline void SetUserCallback(AudioDataCallbackFn a_callback) { m_user_callback = std::move(a_callback); } 

		// Called by miniaudio callback to push samples
		void OnAudioData(const float* a_samples, uint32_t a_frame_count);

		// Static function to retrieve the system's current audio devices
		static std::vector<SAudioDeviceInfo> GetAudioDevices();
	};
}

