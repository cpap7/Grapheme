#include "gpch.h"
#include "GraphemeSTT.h"
#include "../STT/STTEngine.h"

#include <whisper.h>

// Helper function C-C++ interop
// Converts the C-equivalent config to the native C++ version
static Grapheme::SSTTConfig ConvertSSTConfig(const GRAPHEME_STT_Config_t* a_config) {
    Grapheme::SSTTConfig config;

    if (a_config) {
        // TODO: Add more C-config options and convert them here

        // Model config
        config.m_model_config.m_model_path           = a_config->m_model_path ? a_config->m_model_path : "";
        config.m_model_config.m_initial_prompt       = a_config->m_initial_prompt ? a_config->m_initial_prompt : "";
        config.m_model_config.m_language             = a_config->m_language ? a_config->m_language : "en"; // English = default
        config.m_model_config.m_translate_to_english = a_config->m_translate_to_english ? a_config->m_translate_to_english : false; 

        // Hardware config
        config.m_hardware_config.m_thread_count = a_config->m_thread_count ? a_config->m_thread_count : 0; // Auto detect if not specified
        config.m_hardware_config.m_use_gpu      = a_config->m_use_gpu ? a_config->m_use_gpu : true;
    }

    return config;
}

GRAPHEME_API GRAPHEME_STTEngine_Handle_t GRAPHEME_STDCALL GRAPHEME_STT_Create(const GRAPHEME_STT_Config_t* a_config) {
    if (!a_config) {
        printf("[Grapheme] STT Error: Invalid config\n");
        return nullptr;
    }
    
    if (!a_config->m_model_path) {
        printf("[Grapheme] STT Error: Missing model path\n");
        return nullptr;
    }

    try {
        Grapheme::SSTTConfig config = ConvertSSTConfig(a_config);
        auto* stt_engine = new Grapheme::CSTTEngineImpl(config);
        return reinterpret_cast<GRAPHEME_STTEngine_Handle_t>(stt_engine);
    }
    catch (...) {
        return nullptr;
    }


}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_Destroy(GRAPHEME_STTEngine_Handle_t a_engine) {
    if (a_engine) {
        auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);
        delete stt_engine;
    }
}

GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_STT_IsLoaded(GRAPHEME_STTEngine_Handle_t a_engine) {
    if (!a_engine) {
        return -1;
    }

    auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);
    return stt_engine->IsLoaded() ? 1 : 0; // Checks context initialization first 
}

GRAPHEME_API GRAPHEME_STT_Result_t GRAPHEME_STDCALL GRAPHEME_STT_Transcribe(GRAPHEME_STTEngine_Handle_t a_engine, const float* a_audio, int a_sample_count) {
    GRAPHEME_STT_Result_t c_transcript_result = {
        nullptr,    // m_text
        nullptr,    // m_word_probabilities
        0,          // m_word_count
        false       // m_success
    };

    if (!a_engine || !a_audio || a_sample_count <= 0) {
        return c_transcript_result;
    }

    auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);
    try {
        Grapheme::STranscriptResult transcript_result = stt_engine->Transcribe(a_audio, a_sample_count, true);
        if (!transcript_result.Success()) {
            return c_transcript_result;
        }

        // Allocate memory & copy text
        c_transcript_result.m_text = static_cast<char*>(malloc(transcript_result.m_text.size() + 1));
        if (c_transcript_result.m_text) {
            std::memcpy(c_transcript_result.m_text, transcript_result.m_text.c_str(), transcript_result.m_text.size() + 1);
        }

        // Allocate memory & copy word probabilities
        c_transcript_result.m_word_count = static_cast<int>(transcript_result.m_word_probabilities.size());
        if (c_transcript_result.m_word_count > 0) {
            size_t probabilities_size = c_transcript_result.m_word_count * sizeof(float);
            c_transcript_result.m_word_probabilities = static_cast<float*>(malloc(probabilities_size));

            if (c_transcript_result.m_word_probabilities) {
                std::memcpy(c_transcript_result.m_word_probabilities, transcript_result.m_word_probabilities.data(), probabilities_size);
            }
        }

        c_transcript_result.m_success = true;
    }
    catch (...) {
        c_transcript_result.m_success = false;
    }

    return c_transcript_result;
}

GRAPHEME_API const char* GRAPHEME_STDCALL GRAPHEME_STT_TranscribeSimple(GRAPHEME_STTEngine_Handle_t a_engine, const float* a_audio, int a_sample_count) {
    if (!a_engine || !a_audio || a_sample_count <= 0) {
        return nullptr;
    }

    auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);

    Grapheme::STranscriptResult transcript_result = stt_engine->Transcribe(a_audio, a_sample_count, true);
    if (!transcript_result.Success() || transcript_result.m_text.empty()) {
        return nullptr;
    }
    char* transcript_text = static_cast<char*>(malloc(transcript_result.m_text.size() + 1));

    try {    
        if (transcript_text) {
            std::memcpy(transcript_text, transcript_result.m_text.c_str(), transcript_result.m_text.size() + 1);
        }
    }
    catch (...) {
        return nullptr;
    }

    return transcript_text;

}

GRAPHEME_API const char* GRAPHEME_STDCALL GRAPHEME_STT_DetectLanguage(GRAPHEME_STTEngine_Handle_t a_engine, const float* a_audio, int a_sample_count) {
    if (!a_engine || !a_audio || a_sample_count <= 0) {
        return nullptr;
    }

    auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);
    
    Grapheme::SLanguageResult language_result = stt_engine->DetectLanguage(a_audio, a_sample_count);
    if (language_result.m_language_code.empty()) {
        return nullptr;
    }

    char* language_string = static_cast<char*>(malloc(language_result.m_language_code.size() + 1));

    try {
        if (language_string) {
            std::memcpy(language_string, language_result.m_language_code.c_str(), language_result.m_language_code.size() + 1);
        }
    }
    catch (...) {
        return nullptr;
    }

    return language_string;
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_SetProgressCallback(GRAPHEME_STTEngine_Handle_t a_engine, void (*a_callback)(int a_progress)) {
    if (!a_engine) {
        return;
    }

    auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);
    stt_engine->SetProgressCallback(a_callback);
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_Cancel(GRAPHEME_STTEngine_Handle_t a_engine) {
    if (!a_engine) {
        return;
    }

    auto* stt_engine = reinterpret_cast<Grapheme::CSTTEngineImpl*>(a_engine);
    stt_engine->Cancel();
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_FreeString(const char* a_string) {
    if (a_string) {
        free(const_cast<char*>(a_string));
    }
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_FreeFloatArray(float* a_float_array) {
    if (a_float_array) {
        free(a_float_array);
    }
}

GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_FreeResult(GRAPHEME_STT_Result_t* a_result) {
    if (a_result) {
        if (a_result->m_text) {
            free(a_result->m_text);
            a_result->m_text = nullptr;
        }

        if (a_result->m_word_probabilities) {
            free(a_result->m_word_probabilities);
            a_result->m_word_probabilities = nullptr;
        }

        a_result->m_word_count  = 0;
        a_result->m_success     = false;
    }
}

GRAPHEME_API GRAPHEME_STT_Config_t GRAPHEME_STDCALL GRAPHEME_STT_GetDefaultConfig() {
    GRAPHEME_STT_Config_t config = {};
    // TODO: Add more config options and update them here
    config.m_model_path             = nullptr;
    config.m_language               = "en"; // English = default
    config.m_thread_count           = 0;    // 0 = auto detect via whisper.cpp 
    config.m_use_gpu                = true;
    config.m_translate_to_english   = true;
 
    return config;
}

GRAPHEME_API const char* GRAPHEME_STDCALL GRAPHEME_STT_GetVersion() {
    return GRAPHEME_STT_VERSION;
}

// C++ API
#ifdef __cplusplus
namespace Grapheme {
    CSTTEngine::CSTTEngine(const SSTTConfig& a_config) {
        m_engine = std::make_unique<CSTTEngineImpl>(a_config);
	}
	
    CSTTEngine::~CSTTEngine() = default; // We define this here for visibility

    CSTTEngine::CSTTEngine(CSTTEngine&& a_other) noexcept
        : m_engine(std::move(a_other.m_engine)){
	}

	void CSTTEngine::operator=(CSTTEngine&& a_other) noexcept {
        m_engine = std::move(a_other.m_engine);
	}

    bool CSTTEngine::IsLoaded() const {
        return m_engine->IsLoaded();
    }

    STranscriptResult CSTTEngine::Transcribe(const std::vector<float>& a_audio_data) {
        return Transcribe(a_audio_data.data(), static_cast<int>(a_audio_data.size()));
    }

    STranscriptResult CSTTEngine::Transcribe(const float* a_audio_data, int a_sample_count) {
        STranscriptResult result;
        if (!m_engine) {
            result.m_result_code = EResultCode::NotInitialized;
            return result;
        }
        
        if (!a_audio_data || a_sample_count <= 0) {
            result.m_result_code = EResultCode::InvalidParameter;
            return result;
        }

        try {
            result = m_engine->Transcribe(a_audio_data, a_sample_count, true);
            result.m_result_code = (result.m_text.empty()) ? EResultCode::ProcessingFailed : EResultCode::Success;
            return result;
        }
        catch(...) {
            result.m_result_code = EResultCode::InternalError;
            return result;
        }
    }

    std::string CSTTEngine::TranscribeSimple(const float* a_audio_data, int a_sample_count) {
        if (!m_engine || !a_audio_data || a_sample_count <= 0) {
            return "";
        }

        try {
            STranscriptResult result = m_engine->Transcribe(a_audio_data, a_sample_count, false);
            return result.Success() ? result.m_text : "";
        }
        catch (...) {
            return "";
        }
    }

    SLanguageResult CSTTEngine::DetectLanguage(const float* a_audio_data, int a_sample_count) {
        SLanguageResult result;
        if (!m_engine) {
            result.m_result_code = EResultCode::NotInitialized;
            return result;
        }

        if (!a_audio_data || a_sample_count <= 0) {
            result.m_result_code = EResultCode::InvalidParameter;
            return result;
        }

        try {
            result = m_engine->DetectLanguage(a_audio_data, a_sample_count);
            if (result.m_language_code.empty()) {
                result.m_result_code = EResultCode::ProcessingFailed;
            }
        }
        catch (...) {
            result.m_result_code = EResultCode::InternalError;
        }

        return result;
    }

    void CSTTEngine::SetProgressCallback(ProgressCallbackFn a_callback) {
        if (!m_engine) {
            return;
        }

        m_engine->SetProgressCallback(a_callback);
    }

    void CSTTEngine::Cancel() {
        if (!m_engine) {
            return;
        }
        
        m_engine->Cancel();
    }

    std::unique_ptr<CSTTEngine> CSTTEngine::Create(const SSTTConfig& a_config) {
        try {
            return std::make_unique<CSTTEngine>(a_config);
        }
        catch (...) {
            return nullptr;
        }
    }

    SSTTConfig CSTTEngine::GetDefaultConfig() {
        return SSTTConfig();
    }

    std::string CSTTEngine::GetVersion() {
        return GRAPHEME_STT_VERSION;
    }
}
#endif