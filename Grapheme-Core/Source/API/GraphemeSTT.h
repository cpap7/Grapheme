#pragma once
//#include "../Common.h"

#ifdef __cplusplus
    #include <string>
    #include <vector>
    #include <memory>
    #include <functional>

    #include "../STT/STTConfig.h"
    #include "../STT/STTResults.h"
    #include "../STT/STTProgressTracker.h"
#endif

#define GRAPHEME_STT_VERSION "1.0.0"

// C API
#ifdef __cplusplus
extern "C" {
#endif
    // Opaque handle for C
    typedef struct GRAPHEME_STT_Engine_t* GRAPHEME_STT_EngineHandle_t;

    // C-specific config struct (for interop)
    typedef struct {
        const char* m_model_path;
        const char* m_initial_prompt;
        const char* m_language;            // nullptr for auto-detect
        int m_thread_count;                // 0 for auto
        bool m_use_gpu;                    // Default = true
        bool m_translate_to_english;       // Default = false
    } GRAPHEME_STT_Config_t;

    // Transcript result struct (for interop)
    typedef struct {
        char* m_text;                     // Caller must free via GRAPHEME_STT_FreeString
        float* m_word_probabilities;      // Caller must free via GRAPHEME_STT_FreeFloatArray
        int m_word_count;
        bool m_success;
    } GRAPHEME_STT_Result_t;

    // Lifecycle
    GRAPHEME_API GRAPHEME_STT_EngineHandle_t GRAPHEME_STDCALL GRAPHEME_STT_Create(const GRAPHEME_STT_Config_t* a_config);
    GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_Destroy(GRAPHEME_STT_EngineHandle_t a_engine);
    GRAPHEME_API int GRAPHEME_STDCALL GRAPHEME_STT_IsLoaded(GRAPHEME_STT_EngineHandle_t a_engine);

    // Transcription
    GRAPHEME_API GRAPHEME_STT_Result_t GRAPHEME_STDCALL GRAPHEME_STT_Transcribe(GRAPHEME_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count);
    GRAPHEME_API const char* GRAPHEME_STDCALL GRAPHEME_STT_TranscribeSimple(GRAPHEME_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count);
    GRAPHEME_API const char* GRAPHEME_STDCALL GRAPHEME_STT_DetectLanguage(GRAPHEME_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count);

    // Control
    GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_SetProgressCallback(GRAPHEME_STT_EngineHandle_t a_engine, void (*a_callback)(int a_progress));
    GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_Cancel(GRAPHEME_STT_EngineHandle_t a_engine);

    // Memory management
    GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_FreeString(const char* a_string);
    GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_FreeFloatArray(float* a_float_array);
    GRAPHEME_API void GRAPHEME_STDCALL GRAPHEME_STT_FreeResult(GRAPHEME_STT_Result_t* a_result);

    // Utility
    GRAPHEME_API GRAPHEME_STT_Config_t GRAPHEME_STDCALL GRAPHEME_STT_GetDefaultConfig();
    GRAPHEME_API const char* GRAPHEME_STDCALL GRAPHEME_STT_GetVersion();

#ifdef __cplusplus
}
#endif

// C++ API
#ifdef __cplusplus

namespace Grapheme {
    // Forward declaration
    class CSTTEngineImpl;

    // C++ wrapper class for the core engine class
    // Serves as an API call dispatcher & has its own factory
    class GRAPHEME_API CSTTEngine {
    private:
        std::unique_ptr<CSTTEngineImpl> m_engine = nullptr; // Instance

    public:
        // Lifecycle
        CSTTEngine(const SSTTConfig& config);
        ~CSTTEngine();

        // Don't need copies
        CSTTEngine(const CSTTEngine&) = delete;
        void operator=(const CSTTEngine&) = delete;
        
        CSTTEngine(CSTTEngine&& a_other) noexcept;
        void operator=(CSTTEngine&& a_other) noexcept;

        // Status
        bool IsLoaded() const;
        explicit operator bool() const { return IsLoaded(); }

        // Transcription
        STranscriptResult Transcribe(const std::vector<float>& a_audio_data);
        STranscriptResult Transcribe(const float* a_audio_data, int a_sample_count);
        std::string TranscribeSimple(const float* a_audio_data, int a_sample_count);

        // Language detection
        SLanguageResult DetectLanguage(const float* a_audio_data, int a_sample_count);

        // Control
        void SetProgressCallback(ProgressCallbackFn a_callback_function);
        void Cancel();

        // Factory
        static std::unique_ptr<CSTTEngine> Create(const SSTTConfig& a_config);
        
        // Misc
        static SSTTConfig GetDefaultConfig();
        static std::string GetVersion();
    };

} // namespace VoxBox
#endif