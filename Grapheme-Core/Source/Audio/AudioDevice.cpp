#include "gpch.h"
#include "AudioDevice.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace Grapheme {
	
	namespace Utilities {
		static void MiniAudioDataCallback(ma_device* a_device, void* a_output, const void* a_input, uint32_t a_frame_count) {
			// a_output won't be used for device capture
			(void)a_output;
			auto* impl = static_cast<CAudioDeviceImpl*>(a_device->pUserData);
			if (impl && a_input) {
				impl->OnAudioData(static_cast<const float*>(a_input), a_frame_count);
			}
		}
	}

	CAudioDeviceImpl::CAudioDeviceImpl(const SAudioCaptureConfig& a_config) 
		: m_config(a_config), m_ring_buffer((a_config.m_sample_rate * a_config.m_buffer_size_ms) / 1000) {
		Init();
	}

	CAudioDeviceImpl::~CAudioDeviceImpl() {
		Shutdown();
	}

	void CAudioDeviceImpl::Init() {
		// Initialize context

		// For windows, we need wasapi
		ma_backend backends[] = { ma_backend_wasapi };
		bool force_wasapi = (m_config.m_device_type == EDeviceType::Loopback);

		m_context = std::make_unique<ma_context>();
		ma_result ctx_result = force_wasapi 
			? ma_context_init(backends, 1, nullptr, m_context.get()) // Speaker audio bracnh 
			: ma_context_init(nullptr, 0, nullptr, m_context.get()); // Mic input bracnh

		if (ctx_result != MA_SUCCESS) {
			m_context.reset();
			printf("[Grapheme Audio Device] Error: Could not set up context.\n");
			return;
		}

		// Configure capture device
		ma_device_type device_type = (m_config.m_device_type == EDeviceType::Loopback) ? ma_device_type_loopback : ma_device_type_capture;
		ma_device_config device_config	= ma_device_config_init(device_type);
		device_config.capture.format	= ma_format_f32;	// PCM float which matches whisper.cpp input
		device_config.capture.channels	= m_config.m_channels;
		device_config.sampleRate		= m_config.m_sample_rate;
		device_config.dataCallback		= Utilities::MiniAudioDataCallback;
		device_config.pUserData			= this;


		// Select specific device if requested
		// NOTE: empty m_device_id means use system default
		// TODO: Device ID lookup via SAudioCaptureConfig::m_device_id


		m_device = std::make_unique<ma_device>();
		if (ma_device_init(m_context.get(), &device_config, m_device.get()) != MA_SUCCESS) {
			m_device.reset();
			
			printf("[Phoneme Audio Device] Error: Could not set up device.\n");
			return;
		}

		// Pre-size mono downmix buffer to match device's internal buffer size
		if (m_config.m_channels > 1) {
			m_mono_buffer.resize(m_device->capture.internalPeriodSizeInFrames, 0.0f); // Max frame count
		}
	}

	void CAudioDeviceImpl::Shutdown() {
		StopCapture();

		if (m_device) {
			ma_device_uninit(m_device.get());
			m_device.reset();
		}
		if (m_context) {
			ma_context_uninit(m_context.get());
			m_context.reset();
		}
	}

	EResultCode CAudioDeviceImpl::StartCapture() {
		if (!m_device) {
			return EResultCode::NotInitialized;
		}

		m_ring_buffer.Reset();

		if (ma_device_start(m_device.get()) != MA_SUCCESS) {
			return EResultCode::InternalError;
		}

		m_is_capturing.store(true, std::memory_order_release);
		return EResultCode::Success;
	}

	EResultCode CAudioDeviceImpl::StopCapture() {
		if (m_device && m_is_capturing.load(std::memory_order_acquire)) {
			ma_device_stop(m_device.get());
			m_is_capturing.store(false, std::memory_order_release);
		}
		return EResultCode::Success;
	}

	void CAudioDeviceImpl::OnAudioData(const float* a_samples, uint32_t a_frame_count) {
		if (m_config.m_channels == 1) { // Mic input capture or mono audio = 1 channel
			// Audio thread -- push into ring buffer
			m_ring_buffer.Write(a_samples, a_frame_count * m_config.m_channels);
		}
		else {
			// Downmix stereo audio to mono (avg of all channels per frame)
			for (uint32_t i{}; i < a_frame_count; ++i) {
				float sum = 0.0f;
				for (uint32_t channel{}; channel < m_config.m_channels; ++channel) {
					sum += a_samples[i * m_config.m_channels + channel];
				}
				m_mono_buffer[i] = sum / static_cast<float>(m_config.m_channels);
			}
			m_ring_buffer.Write(m_mono_buffer.data(), a_frame_count);
		}
		// If consumer set a callback, we forward it immediately
		// NOTE: The callback must be fast considering this runs on the audio thread
		if (m_user_callback) {
			m_user_callback(a_samples, a_frame_count * m_config.m_channels);
		}
	}

	std::vector<SAudioDeviceInfo> CAudioDeviceImpl::GetAudioDevices() {
		std::vector<SAudioDeviceInfo> devices;
		ma_context context;
		if (ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS) {
			return devices;
		}
		ma_device_info* capture_infos = nullptr;
		uint32_t capture_count = 0;

		if (ma_context_get_devices(&context, nullptr, nullptr, &capture_infos, &capture_count) == MA_SUCCESS) {
			for (uint32_t i{}; i < capture_count; ++i) {
				// Populate info struct
				SAudioDeviceInfo info;
				info.m_name			= capture_infos[i].name;
				info.m_is_default	= capture_infos[i].isDefault != 0;
				info.m_device_id	= std::to_string(i); // We store raw ID bytes as hex string for easier lookup later


				devices.push_back(std::move(info));
			}
		}
		ma_context_uninit(&context);
		return devices;
	}
}