#pragma once

// Precompiled header -- used internally for .cpp files

//  Platform Detection 
#ifdef _WIN32
	#define GRAPHEME_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
#elif defined(__APPLE__)
	#define VB_PLATFORM_MACOS
#elif defined(__linux__)
	#define VB_PLATFORM_LINUX
#endif

// Platform-Specific Headers
#ifdef GRAPHEME_PLATFORM_WINDOWS
	#include <Windows.h>
#elif defined(GRAPHEME_PLATFORM_MACOS)
	#include <mach-o/dyld.h>
	#include <climits>  // For PATH_MAX
#elif defined(GRAPHEME_PLATFORM_LINUX)
	#include <climits>  // For PATH_MAX
#endif

// Standard Library 
#ifdef __cplusplus
	#include <cstdint>
	#include <cstdbool>
#else
	#include <stdint.h>
	#include <stdbool.h>
#endif

#include <cassert>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "Core/PlatformDetection.h"
#include "Core/ResultCodes.h"

// DLL Macros
#ifdef GRAPHEME_PLATFORM_WINDOWS
	#define GRAPHEME_DLL_EXPORT __declspec(dllexport)
	#define GRAPHEME_DLL_IMPORT __declspec(dllimport)
	#define GRAPHEME_STDCALL __stdcall
#else
	#define GRAPHEME_DLL_EXPORT
	#define GRAPHEME_DLL_IMPORT
	#define GRAPHEME_STDCALL
#endif

#ifdef GRAPHEME_EXPORT_API
	#define GRAPHEME_API GRAPHEME_DLL_EXPORT
#else
	#define GRAPHEME_API GRAPHEME_DLL_IMPORT
#endif