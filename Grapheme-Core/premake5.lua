-- premake5.lua
project "Grapheme-Core"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "Off" -- /MD 
    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")


    -- Precompiled header
    pchheader "gpch.h"
    pchsource "Source/gpch.cpp"

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs {
        "Source",
        "Vendor/whisper.cpp/include",                -- whisper.h
        "Vendor/whisper.cpp/ggml/include",           -- ggml headers
        "Vendor/miniaudio",
        "Vendor/json"
    }

    libdirs {
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/src/%{cfg.buildcfg}",                      -- whisper.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/%{cfg.buildcfg}",                 -- ggml.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-base/%{cfg.buildcfg}",       -- ggml-base.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-cpu/%{cfg.buildcfg}",        -- ggml-cpu.lib
        --"Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-vulkan/%{cfg.buildcfg}",     -- ggml-vulkan.lib

    }

    links {
        "whisper",
        "ggml",
        "ggml-base",
        "ggml-cpu",
        --"ggml-vulkan",
        --"%{Library.Vulkan}"
    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        buildoptions { "/utf-8", "/Wall" }
        defines { 
            "_WINDOWS",
            "_USRDLL",
            "GRAPHEME_EXPORT_API" -- Export dll symbols
        }
        

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }
        links {
            "msvcprtd",
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        defines { "NDEBUG" }
        links {
            "msvcprt",
        }
