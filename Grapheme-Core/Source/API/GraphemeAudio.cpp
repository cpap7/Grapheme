#include "gpch.h"
#include "GraphemeAudio.h"
#include "../Audio/AudioDevice.h"

#ifdef __cplusplus
namespace Grapheme {
	CAudioDevice::CAudioDevice(const SAudioCaptureConfig& a_config) {
		m_audio_device_impl = std::make_unique<CAudioDeviceImpl>(a_config);
	}

	CAudioDevice::~CAudioDevice() = default;

	CAudioDevice::CAudioDevice(CAudioDevice&& a_other) noexcept
		: m_audio_device_impl(std::move(a_other.m_audio_device_impl)) {

	}

	void CAudioDevice::operator=(CAudioDevice&& a_other) noexcept {
		m_audio_device_impl = std::move(a_other.m_audio_device_impl);
	}

	EResultCode CAudioDevice::StartCapture() {
		return m_audio_device_impl ? m_audio_device_impl->StartCapture() : EResultCode::NotInitialized;
	}

	EResultCode CAudioDevice::StopCapture() {
		return m_audio_device_impl ? m_audio_device_impl->StopCapture() : EResultCode::NotInitialized;
	}

	bool CAudioDevice::IsCapturing() const {
		return m_audio_device_impl ? m_audio_device_impl->IsCapturing() : false;
	}

	bool CAudioDevice::IsReady() const {
		return m_audio_device_impl ? m_audio_device_impl->IsReady() : false;
	}

	void CAudioDevice::SetAudioDataCallback(AudioDataCallbackFn a_callback) {
		if (m_audio_device_impl) {
			m_audio_device_impl->SetUserCallback(a_callback);
		}
	}

	std::vector<SAudioDeviceInfo> CAudioDevice::GetAudioDevices() {
		return CAudioDeviceImpl::GetAudioDevices();
	}

	std::unique_ptr<CAudioDevice> CAudioDevice::Create(const SAudioCaptureConfig& a_config) {
		try {
			return std::make_unique<CAudioDevice>(a_config);
		}
		catch (...) {
			return nullptr;
		}
	}
}

#endif