workspace "Mule"
    configurations { "Debug", "Release" }
    architecture "x64"
    startproject "Mule Editor"

    dir = os.getcwd();

    includes = {
        dir .. "/Submodules/imgui",
        dir .. "/Submodules/glfw/include",
        dir .. "/Submodules/spdlog/include",
        dir .. "/Submodules/stb",
        dir .. "/Submodules/entt/single_include",
        dir .. "/Submodules/glm",
        dir .. "/Submodules/yaml/include",
        dir .. "/Submodules/nativefiledialog/src/include",
        dir .. "/Submodules/Assimp/include",
        dir .. "/Submodules/ImGuizmo",
        "%VULKAN_SDK%/Include"
    } 

    debugLibs = {
        "%VULKAN_SDK%/Lib/shadercd.lib",
        "%VULKAN_SDK%/Lib/shaderc_combinedd.lib",
        "%VULKAN_SDK%/Lib/shaderc_utild.lib",
        dir .. "/Submodules/Assimp/lib/Debug/assimp-vc143-mtd.lib",
        dir .. "/Submodules/Assimp/lib/Debug/dracod.lib",
        dir .. "/Submodules/Assimp/lib/Debug/dracod.lib",
        dir .. "/Submodules/Assimp/contrib/zlib/Debug/zlibstaticd.lib",
    }

    releaseLibs = {
        "%VULKAN_SDK%/Lib/shaderc.lib",
        "%VULKAN_SDK%/Lib/shaderc_combined.lib",
        "%VULKAN_SDK%/Lib/shaderc_util.lib",
        dir .. "/Submodules/Assimp/lib/Release/assimp-vc143-mt.lib",
        dir .. "/Submodules/Assimp/lib/Release/draco.lib"
    }

    libs = {
        "%VULKAN_SDK%/Lib/vulkan-1.lib"
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
        "YAML_CPP_STATIC_DEFINE",
        "VK_USE_PLATFORM_WIN32_KHR", --TODO add to windows filter
        "GLFW_EXPOSE_NATIVE_WIN32", --TODO add to windows filter
        "ASSIMP_BUILD_NO_EXPORT"
    }
    
    -- Submodule
    group "Submodules"
        include "Submodules/imgui/premake5.lua"
        include "Submodules/glfw/premake5.lua"
        include "Submodules/spdlog/premake5.lua"
        include "Submodules/yaml/yaml.lua"
        include "Submodules/nativefiledialog/nativefiledialog.lua"
        include "Submodules/ImGuizmo/imguizmo.lua"
    group ""
    -- Projects
    include "Mule Editor/editor.lua"
    include "Mule/mule.lua"
