#pragma once
#include "../Common.h"
#include "GraphemeSTT.h"

#ifdef __cplusplus
	#include <memory>
	#include <string>
	#include <vector>
	//#include <cstdbool>

	#include "../Serialization/SerializationConfig.h"
#else
	#include <stdint.h>
	#include <stdbool.h>
#endif

// C API
#ifdef __cplusplus
extern "C" {
#endif

	typedef struct GRAPHEME_Serializer_t* GRAPHEME_Serializer_Handle_t;
	
	typedef struct {
		int m_indent_width;				// Default = 4
		bool m_pretty_print;			// Default = true
		bool m_include_timestamps;		// Default = true
		bool m_include_confidence;		// Default = true
		bool m_include_metadata;		// Default = true
	} GRAPHEME_Serialization_Config_t;

	// Lifecycle
	GRAPHEME_API GRAPHEME_Serializer_Handle_t GRAPHEME_STDCALL GRAPHEME_Serializer_Create(const GRAPHEME_Serialization_Config_t* a_config);
	GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_Serializer_Destroy(GRAPHEME_Serializer_Handle_t a_serializer_handle);

	// File I/O
	GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_Serializer_SaveToFile(GRAPHEME_Serializer_Handle_t a_serializer_handle, const char* a_path, const GRAPHEME_STT_Result_t* a_result);
	GRAPHEME_API GRAPHEME_STT_Result_t GRAPHEME_STDCALL GRAPHEME_Serializer_LoadFromFile(GRAPHEME_Serializer_Handle_t a_serializer_handle, const char* a_path);

	// Utility
	GRAPHEME_API GRAPHEME_Serialization_Config_t GRAPHEME_STDCALL GRAPHEME_Serializer_GetDefaultConfig();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace Grapheme {
	// Forward declarations
	class CTranscriptSerializerImpl;

	class GRAPHEME_API CTranscriptSerializer {
	private:
		std::unique_ptr<CTranscriptSerializerImpl> m_impl;

	public:
		explicit CTranscriptSerializer(const SSerializationConfig& a_config);
		~CTranscriptSerializer();

		// File I/O
		EResultCode SaveToFile(const std::string& a_path, const STranscriptResult& a_result) const;
		EResultCode SaveToFile(const std::string& a_path, const std::vector<STranscriptResult>& a_results) const;

		// Deserialization
		STranscriptResult LoadFromFile(const std::string& a_path) const;

		// Factory
		static std::unique_ptr<CTranscriptSerializer> Create(const SSerializationConfig& a_config);

		// Misc
		static SSerializationConfig GetDefaultConfig();
	};
}


#endif