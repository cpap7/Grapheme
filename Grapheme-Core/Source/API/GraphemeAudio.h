#pragma once

#include "../Common.h"

#ifdef __cplusplus
	#include <memory>
	#include <vector>
	#include <cstdbool>

	#include "../Audio/AudioConfig.h"
#else // C-specific includes
	#include <stdint.h>
	#include <stdbool.h>
#endif

// C API
#ifdef __cplusplus
extern "C" {
#endif
	typedef enum {
		GRAPHEME_DEVICE_TYPE_CAPTURE = 1,
		GRAPHEME_DEVICE_TYPE_LOOPBACK = 2
	} GRAPHEME_DeviceType_t;

	typedef struct GRAPHEME_AudioDevice_t* GRAPHEME_AudioDevice_Handle_t;

	typedef struct {
		const char* m_device_id;	// Empty = default
		uint32_t m_sample_rate;		// 16 kHz for whisper.cpp
		uint32_t m_channels;		// 1 = mono, 2 = stereo
		uint32_t m_buffer_size_ms;	// Ring buffer duration
		GRAPHEME_DeviceType_t m_device_type;
	} GRAPHEME_AudioDevice_Config_t;

	typedef struct {
		const char* m_device_name;	// Device display name
		const char* m_device_id;	// Platform-specific ID
		bool m_is_default;			// true = default device
	} GRAPHEME_AudioDevice_Info_t;

	typedef struct {
		GRAPHEME_AudioDevice_Info_t* m_devices;
		int m_device_count;
	} GRAPHEME_AudioDevice_List_t;

	// Lifecycle
	GRAPHEME_API GRAPHEME_AudioDevice_Handle_t GRAPHEME_STDCALL GRAPHEME_AudioDevice_Create(const GRAPHEME_AudioDevice_Config_t* a_config);
	GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_AudioDevice_Destroy(GRAPHEME_AudioDevice_Handle_t a_device_handle);
	
	// Audio capture control
	GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_StartCapture(GRAPHEME_AudioDevice_Handle_t a_device_handle);
	GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_StopCapture(GRAPHEME_AudioDevice_Handle_t a_device_handle);

	// Status getters
	GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_IsReady(GRAPHEME_AudioDevice_Handle_t a_device_handle);
	GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_IsCapturing(GRAPHEME_AudioDevice_Handle_t a_device_handle);

	// Callback setter
	GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_AudioDevice_SetCallback(GRAPHEME_AudioDevice_Handle_t a_device_handle, void (*a_callback)(const float* a_samples, uint32_t a_sample_count));

	// Generators
	GRAPHEME_API GRAPHEME_AudioDevice_List_t GRAPHEME_STDCALL GRAPHEME_AudioDevice_GetDeviceList();
	GRAPHEME_API GRAPHEME_AudioDevice_Config_t GRAPHEME_STDCALL GRAPHEME_AudioDevice_GetDefaultConfig();

	// Memory management
	GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_AudioDevice_FreeDeviceList(GRAPHEME_AudioDevice_List_t* a_list);


#ifdef __cplusplus
}
#endif

// C++ API
#ifdef __cplusplus
namespace Grapheme {
	// Forward declarations
	class CAudioDeviceImpl;

	class GRAPHEME_API CAudioDevice {
	private:
		std::unique_ptr<CAudioDeviceImpl> m_audio_device_impl = nullptr;
		
	public:
		explicit CAudioDevice(const SAudioCaptureConfig& a_config);
		~CAudioDevice();

		CAudioDevice(const CAudioDevice& a_other) = delete;
		void operator=(const CAudioDevice& a_other) = delete;

		CAudioDevice(CAudioDevice&& a_other) noexcept;
		void operator=(CAudioDevice&& a_other) noexcept;

		// Lifecycle
		EResultCode StartCapture();
		EResultCode StopCapture();

		// Getters
		bool IsCapturing() const;
		bool IsReady() const;

		// Callback
		void SetAudioDataCallback(AudioDataCallbackFn a_callback);
	
		// Utilities
		static std::vector<SAudioDeviceInfo> GetAudioDevices();

		// Factory
		static std::unique_ptr<CAudioDevice> Create(const SAudioCaptureConfig& a_config);
	};
}
#endif