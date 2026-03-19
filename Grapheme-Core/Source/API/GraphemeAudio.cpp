#include "gpch.h"
#include "GraphemeAudio.h"
#include "../Audio/AudioDevice.h"

// Helper function for converting device type enums (C -> C++)
static Grapheme::EDeviceType ConvertDeviceType(GRAPHEME_DeviceType_t a_device_type) {
	switch (a_device_type) {
		case GRAPHEME_DEVICE_TYPE_CAPTURE:	return Grapheme::EDeviceType::Capture;
		case GRAPHEME_DEVICE_TYPE_LOOPBACK:	return Grapheme::EDeviceType::Loopback;
	}

	return Grapheme::EDeviceType::Default;
}

// Helper function for converting the configs (C -> C++)
static Grapheme::SAudioCaptureConfig ConvertAudioDeviceConfig(const GRAPHEME_AudioDevice_Config_t* a_config) {
	Grapheme::SAudioCaptureConfig cpp_config;
	
	if (a_config) {
		cpp_config.m_device_id		= a_config->m_device_id ? a_config->m_device_id : "";
		cpp_config.m_sample_rate	= a_config->m_sample_rate ? a_config->m_sample_rate : 16000;		// Default = 16 kHz for whisper.cpp
		cpp_config.m_channels		= a_config->m_channels ? a_config->m_channels : 1;					// Default = mono
		cpp_config.m_buffer_size_ms = a_config->m_buffer_size_ms ? a_config->m_buffer_size_ms : 500;
		cpp_config.m_device_type	= ConvertDeviceType(a_config->m_device_type);
	}
	return cpp_config;
}

GRAPHEME_API GRAPHEME_AudioDevice_Handle_t GRAPHEME_STDCALL GRAPHEME_AudioDevice_Create(const GRAPHEME_AudioDevice_Config_t* a_config) {
	if (!a_config) {
		printf("[Grapheme] Audio Error: Invalid config\n");
		return nullptr;
	}
	try {
		Grapheme::SAudioCaptureConfig config = ConvertAudioDeviceConfig(a_config);
		auto* device = new Grapheme::CAudioDeviceImpl(config);
		return reinterpret_cast<GRAPHEME_AudioDevice_Handle_t>(device);
	}
	catch (...) {
		return nullptr;
	}
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_AudioDevice_Destroy(GRAPHEME_AudioDevice_Handle_t a_device_handle) {
	if (a_device_handle) {
		auto* device = reinterpret_cast<Grapheme::CAudioDeviceImpl*>(a_device_handle);
		delete device;
	}
}

GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_StartCapture(GRAPHEME_AudioDevice_Handle_t a_device_handle) {
	if (!a_device_handle) {
		return GRAPHEME_RESULT_NOT_INITIALIZED;
	}

	auto* device = reinterpret_cast<Grapheme::CAudioDeviceImpl*>(a_device_handle);
	Grapheme::EResultCode result = device->StartCapture();
	return static_cast<int>(result);
}

GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_StopCapture(GRAPHEME_AudioDevice_Handle_t a_device_handle) {
	if (!a_device_handle) {
		return GRAPHEME_RESULT_NOT_INITIALIZED;
	}

	auto* device = reinterpret_cast<Grapheme::CAudioDeviceImpl*>(a_device_handle);
	Grapheme::EResultCode result = device->StopCapture();
	return static_cast<int>(result);
}

GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_IsReady(GRAPHEME_AudioDevice_Handle_t a_device_handle) {
	if (!a_device_handle) {
		return 0;
	}

	auto* device = reinterpret_cast<Grapheme::CAudioDeviceImpl*>(a_device_handle);
	return device->IsReady() ? 1 : 0;
}

GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_IsCapturing(GRAPHEME_AudioDevice_Handle_t a_device_handle) {
	if (!a_device_handle) {
		return 0;
	}

	auto* device = reinterpret_cast<Grapheme::CAudioDeviceImpl*>(a_device_handle);
	return device->IsCapturing() ? 1 : 0;
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_AudioDevice_SetCallback(GRAPHEME_AudioDevice_Handle_t a_device_handle, void (*a_callback)(const float* a_samples, uint32_t a_sample_count)) {
	if (!a_device_handle) {
		return;
	}

	auto* device = reinterpret_cast<Grapheme::CAudioDeviceImpl*>(a_device_handle);
	device->SetUserCallback(a_callback);
}

GRAPHEME_API GRAPHEME_AudioDevice_List_t GRAPHEME_STDCALL GRAPHEME_AudioDevice_GetDeviceList() {
	GRAPHEME_AudioDevice_List_t list = { nullptr, 0 };
	
	try {
		std::vector<Grapheme::SAudioDeviceInfo> devices = Grapheme::CAudioDeviceImpl::GetAudioDevices();
		
		if (devices.empty()) {
			return list;
		}

		list.m_devices = static_cast<GRAPHEME_AudioDevice_Info_t*>(malloc(static_cast<int>(devices.size()) * sizeof(GRAPHEME_AudioDevice_Info_t)));
		list.m_device_count = static_cast<int>(devices.size());
		
		if (!list.m_devices) {
			list.m_device_count = 0;
			return list;
		}

		for(int i{}; i < list.m_device_count; ++i) {
			// Allocate memory, then copy device name
			list.m_devices[i].m_device_name = static_cast<char*>(malloc(devices[i].m_name.size() + 1));
			if (list.m_devices[i].m_device_name) {
				std::memcpy(const_cast<char*>(list.m_devices[i].m_device_name), devices[i].m_name.c_str(), devices[i].m_name.size() + 1);
			}

			// Allocate memory, then copy device id
			list.m_devices[i].m_device_id = static_cast<char*>(malloc(devices[i].m_device_id.size() + 1));
			if (list.m_devices[i].m_device_id) {
				std::memcpy(const_cast<char*>(list.m_devices[i].m_device_id), devices[i].m_device_id.c_str(), devices[i].m_device_id.size() + 1);
			}

			list.m_devices[i].m_is_default = devices[i].m_is_default ? 1 : 0;
		}
	}
	catch(...) {
		GRAPHEME_AudioDevice_FreeDeviceList(&list);
	}

	return list;
}

GRAPHEME_API GRAPHEME_AudioDevice_Config_t GRAPHEME_STDCALL GRAPHEME_AudioDevice_GetDefaultConfig() {
	GRAPHEME_AudioDevice_Config_t config = {};
	config.m_device_id		= "";
	config.m_sample_rate	= 16000;
	config.m_channels		= 1;
	config.m_buffer_size_ms = 500;
	config.m_device_type	= GRAPHEME_DEVICE_TYPE_LOOPBACK;
	
	return config;
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_AudioDevice_FreeDeviceList(GRAPHEME_AudioDevice_List_t* a_list) {
	if (!a_list || !a_list->m_devices) {
		return;
	}

	for (int i{}; i < a_list->m_device_count; ++i) {
		if (a_list->m_devices[i].m_device_name) {
			free(const_cast<char*>(a_list->m_devices[i].m_device_name));
		}
		if(a_list->m_devices[i].m_device_id) {
			free(const_cast<char*>(a_list->m_devices[i].m_device_id));
		}
	}

	free(a_list->m_devices);
	a_list->m_devices = nullptr;
	a_list->m_device_count = 0;
}


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