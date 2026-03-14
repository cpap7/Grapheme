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
