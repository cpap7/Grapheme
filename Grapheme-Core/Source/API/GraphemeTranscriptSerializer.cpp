#include "gpch.h"
#include "GraphemeTranscriptSerializer.h"

#include "../Serialization/TranscriptSerializer.h" // C++ impl
//#include "../STT/STTResults.h"

// Helper to convert configs (C -> C++)
static Grapheme::SSerializationConfig ConvertSerializerConfig(const GRAPHEME_Serialization_Config_t* a_config) {
	Grapheme::SSerializationConfig cpp_config;
	
	if (a_config) {
		cpp_config.m_indent_width		= a_config->m_indent_width > 0 ? a_config->m_indent_width : 4;
		cpp_config.m_pretty_print		= a_config->m_pretty_print != 0;
		cpp_config.m_include_timestamps = a_config->m_include_timestamps != 0;
		cpp_config.m_include_confidence = a_config->m_include_confidence != 0;
		cpp_config.m_include_metadata	= a_config->m_include_metadata != 0;
	}

	return cpp_config;
}

// Helper to convert transcript result structs (C -> C++)
static Grapheme::STranscriptResult ConvertTranscriptResult(const GRAPHEME_STT_Result_t* a_result) {
	Grapheme::STranscriptResult cpp_result;
	
	if (a_result) {
		cpp_result.m_text = a_result->m_text ? a_result->m_text : "";
		cpp_result.m_result_code = a_result->m_success ? Grapheme::EResultCode::Success : Grapheme::EResultCode::ProcessingFailed;
		
		if (a_result->m_word_probabilities && a_result->m_word_count > 0) {
			cpp_result.m_word_probabilities.assign(a_result->m_word_probabilities, a_result->m_word_probabilities + a_result->m_word_count);
		}
	}

	return cpp_result;
}

// Helper to malloc a copy of std::string for C
static char* AllocCString(const std::string& a_string) {
	char* c_string = static_cast<char*>(malloc(a_string.size() + 1));
	if (c_string) {
		std::memcpy(c_string, a_string.c_str(), a_string.size() + 1);
	}

	return c_string;
}

GRAPHEME_API GRAPHEME_Serializer_Handle_t GRAPHEME_STDCALL GRAPHEME_Serializer_Create(const GRAPHEME_Serialization_Config_t* a_config) {
	try {
		Grapheme::SSerializationConfig config = a_config ? ConvertSerializerConfig(a_config) : Grapheme::SSerializationConfig{};
		auto* serializer = new Grapheme::CTranscriptSerializerImpl(config);
		return reinterpret_cast<GRAPHEME_Serializer_Handle_t>(serializer);
	}
	catch (...) {
		return nullptr;
	}

}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_Serializer_Destroy(GRAPHEME_Serializer_Handle_t a_serializer_handle) {
	if (a_serializer_handle) {
		auto* serializer = reinterpret_cast<Grapheme::CTranscriptSerializerImpl*>(a_serializer_handle);
		delete serializer;
	}
}


GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_Serializer_SaveToFile(GRAPHEME_Serializer_Handle_t a_serializer_handle, const char* a_path, const GRAPHEME_STT_Result_t* a_result) {
	if (!a_serializer_handle || !a_path || !a_result) {
		return GRAPHEME_RESULT_INVALID_PARAMETER;
	}
	try {
		auto* serializer = reinterpret_cast<Grapheme::CTranscriptSerializerImpl*>(a_serializer_handle);
		Grapheme::STranscriptResult cpp_result = ConvertTranscriptResult(a_result);
		Grapheme::EResultCode result = serializer->SaveToFile(a_path, cpp_result);
		return static_cast<int>(result);
	}
	catch (...) {
		return GRAPHEME_RESULT_INTERNAL_ERROR;
	}
}

GRAPHEME_API GRAPHEME_STT_Result_t GRAPHEME_STDCALL GRAPHEME_Serializer_LoadFromFile(GRAPHEME_Serializer_Handle_t a_serializer_handle, const char* a_path) {
	GRAPHEME_STT_Result_t c_result = { nullptr, nullptr, 0, false };
	
	if (!a_serializer_handle || !a_path) {
		return c_result;
	}

	try {
		auto* serializer = reinterpret_cast<Grapheme::CTranscriptSerializerImpl*>(a_serializer_handle);
		Grapheme::STranscriptResult cpp_result = serializer->LoadFromFile(a_path);
		
		c_result.m_text = AllocCString(cpp_result.m_text);
		c_result.m_word_count = static_cast<int>(cpp_result.m_word_probabilities.size());
		if (c_result.m_word_count > 0) {
			size_t size = c_result.m_word_count * sizeof(float);
			c_result.m_word_probabilities = static_cast<float*>(malloc(size));
			if (c_result.m_word_probabilities) {
				std::memcpy(c_result.m_word_probabilities, cpp_result.m_word_probabilities.data(), size);
			}
		}
		c_result.m_success = true;
	}
	catch (...) {
		c_result.m_success = false;
	}

	return c_result;
}

GRAPHEME_API GRAPHEME_Serialization_Config_t GRAPHEME_STDCALL GRAPHEME_Serializer_GetDefaultConfig() {
	GRAPHEME_Serialization_Config_t config = {};
	config.m_indent_width		= 4;
	config.m_pretty_print		= true;
	config.m_include_timestamps = true;
	config.m_include_confidence = true;
	config.m_include_metadata	= true;

	return config;
}

#ifdef __cplusplus
namespace Grapheme {

	CTranscriptSerializer::CTranscriptSerializer(const SSerializationConfig& a_config) 
		: m_impl(std::make_unique<CTranscriptSerializerImpl>(a_config)) {
	}

	CTranscriptSerializer::~CTranscriptSerializer() = default;


	EResultCode CTranscriptSerializer::SaveToFile(const std::string& a_path, const STranscriptResult& a_result) const {
		return m_impl ? m_impl->SaveToFile(a_path, a_result) : EResultCode::NotInitialized;
	}

	EResultCode CTranscriptSerializer::SaveToFile(const std::string& a_path, const std::vector<STranscriptResult>& a_results) const {
		return m_impl ? m_impl->SaveToFile(a_path, a_results) : EResultCode::NotInitialized;
	}

	STranscriptResult CTranscriptSerializer::LoadFromFile(const std::string& a_path) const {
		return m_impl ? m_impl->LoadFromFile(a_path) : STranscriptResult{};
	}

	std::unique_ptr<CTranscriptSerializer> CTranscriptSerializer::Create(const SSerializationConfig& a_config) {
		try {
			return std::make_unique<CTranscriptSerializer>(a_config);
		}
		catch (...) {
			return nullptr;
		}
	}

	SSerializationConfig CTranscriptSerializer::GetDefaultConfig() {
		return SSerializationConfig();
	}
}


#endif