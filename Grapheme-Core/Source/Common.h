#pragma once

#ifdef _WIN32 // Windows x86
    #define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
    #ifdef _MSC_VER
        #define NOMINMAX
        #include <Windows.h>
    #endif // _MSC_VER

    #include <fcntl.h>
    #include <io.h>

    #define GRAPHEME_PLATFORM_WINDOWS
    //#define GRAPHEME_STDCALL __stdcall

// Have to check __ANDROID__ before __linux__ 
// since it's based on the Linux kernel 
// and has it defined
    #elif defined(__ANDROID__)
        #define GRAPHEME_PLATFORM_ANDROID
        #define GRAPHEME_STDCALL

    #elif defined(__linux__)
        #define GRAPHEME_PLATFORM_LINUX   
        #define GRAPHEME_STDCALL 

    #elif defined(__APPLE__) || defined(__MACH__)
        #include <mach-o/dyld.h>
        #include <TargetConditionals.h>
/* TARGET_OS_MAC exists on all platforms,
* so all different Apple platforms must be
* checked to ensure it's running on MACOS, not
* IOS or some other Apple platform
*/
    #if TARGET_IPHONE_SIMULATOR == 1
        #define GRAPHEME_PLATFORM_IOS_SIMULATOR
        #define GRAPHEME_STDCALL 

    #elif TARGET_OS_IPHONE == 1
        #define GRAPHEME_PLATFORM_IOS
        #define GRAPHEME_STDCALL 

    #elif TARGET_OS_MAC == 1
        #define GRAPHEME_PLATFORM_MACOS
        #define GRAPHEME_STDCALL

    #else
        #error "Unknown Apple platform!"
    #endif 
#endif // _WIN32

// DLL macros
#ifdef GRAPHEME_PLATFORM_WINDOWS
    // declspec
    #ifdef GRAPHEME_EXPORT_API
        #define GRAPHEME_API __declspec(dllexport)
    #else
        #define GRAPHEME_API __declspec(dllimport)
    #endif
    // std call
    #define GRAPHEME_STDCALL __stdcall
#else
    #define GRAPHEME_API
    #define GRAPHEME_STDCALL
#endif // GRAPHEME_PLATFORM_WINDOWS


#ifdef __cplusplus
#include <cstdint>
#include <string>

namespace Grapheme {
    // Common result codes
    enum class EResultCode : uint8_t {
        Unknown = 0,

        Success,
        NotInitialized,
        InvalidParameter,
        ModelLoadFailed,
        ProcessingFailed,
        Cancelled,
        OutOfMemory,
        InternalError

    };

    inline std::string ResultCodeToString(EResultCode a_result_code) {
        switch (a_result_code) {
            case EResultCode::Success:          return "SUCCESS";
            case EResultCode::NotInitialized:   return "NOT INITIALIZED";
            case EResultCode::InvalidParameter: return "INVALID PARAMETER(S)";
            case EResultCode::ModelLoadFailed:  return "MODEL LOAD FAILED";
            case EResultCode::ProcessingFailed: return "PROCESSING FAILED";
            case EResultCode::Cancelled:        return "CANCELLED";
            case EResultCode::OutOfMemory:      return "OUT OF MEMORY";
            case EResultCode::InternalError:    return "INTERNAL ERROR";
        }

        return "UNKNOWN RESULT CODE";
    }

    inline EResultCode StringToResultCode(const std::string& a_string) {
        if (a_string == "SUCCESS") { return EResultCode::Success; }
        if (a_string == "NOT INITIALIZED") { return EResultCode::NotInitialized; }
        if (a_string == "INVALID PARAMETER(S)") { return EResultCode::InvalidParameter; }
        if (a_string == "MODEL LOAD FAILED") { return EResultCode::ModelLoadFailed; }
        if (a_string == "PROCESSING FAILED") { return EResultCode::ProcessingFailed; }
        if (a_string == "CANCELLED") { return EResultCode::Cancelled; }
        if (a_string == "OUT OF MEMORY") { return EResultCode::OutOfMemory; }
        if (a_string == "INTERNAL ERROR") { return EResultCode::InternalError; }

        return EResultCode::Unknown;
    }
}
#endif