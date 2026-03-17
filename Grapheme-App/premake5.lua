-- premake5.lua
project "Grapheme-App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    
    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")

    files {
        "Source/**.h",
        "Source/**.c",
        "Source/**.hpp",
        "Source/**.cpp"
    }

    includedirs {
        "../Grapheme-Core/Source/API",
    }

    libdirs {
        "../Grapheme-Core/bin/%{cfg.buildcfg}-%{cfg.architecture}",
    }

    links {
        "Grapheme-Core",
    }

    postbuildcommands {
        '{COPYFILE} "../Grapheme-Core/bin/%{cfg.buildcfg}-%{cfg.architecture}/Grapheme-Core.dll" "%{cfg.targetdir}"',
    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        defines { "_WINDOWS" }

    filter "configurations:Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }