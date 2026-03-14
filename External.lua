VULKAN_SDK = os.getenv("VULKAN_SDK")
VCPKG_ROOT = os.getenv("VCPKG_ROOT") or "C:/vcpkg"

-- Dependency folders relative to the root folder
IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["VCPKG"] = "%{VCPKG_ROOT}/installed/x64-windows/include" -- Zlib, Curl, OpenSSL

-- Binary libs relative to the root folder
LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VCPKG"] = "%{VCPKG_ROOT}/installed/x64-windows/lib" -- Zlib, Curl, OpenSSL

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"


group "Applications"
   include "Grapheme-App"
group ""

group "Core"
	include "Grapheme-Core"
group ""

