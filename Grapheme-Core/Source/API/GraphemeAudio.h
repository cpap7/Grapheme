#pragma once

#ifdef __cplusplus
	#include <memory>
	#include <vector>
	
	#include "../Core/ResultCodes.h"
	#include "../Core/PlatformDetection.h"

	#include "../Audio/AudioConfig.h"
#else // C-specific includes

#endif

// C API
#ifdef __cplusplus
extern "C" {
#endif
	typedef struct GRAPHEME_AudioDevice_t* GRAPHEME_AudioDevice_Handle_t;

	// TODO:
	//GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_StartCapture(GRAPHEME_AudioDevice_Handle_t a_device_handle);
	//GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_AudioDevice_StopCapture(GRAPHEME_AudioDevice_Handle_t a_device_handle);


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