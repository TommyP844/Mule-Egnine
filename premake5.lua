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
        dir .. "/Submodules/tinygltf",
        dir .. "/Submodules/ImGuizmo",
        "%VULKAN_SDK%/Include"
    } 

    debugLibs = {
        "%VULKAN_SDK%/Lib/shadercd.lib",
        "%VULKAN_SDK%/Lib/shaderc_combinedd.lib",
        "%VULKAN_SDK%/Lib/shaderc_utild.lib"
    }

    releaseLibs = {
        "%VULKAN_SDK%/Lib/shaderc.lib",
        "%VULKAN_SDK%/Lib/shaderc_combined.lib",
        "%VULKAN_SDK%/Lib/shaderc_util.lib"
    }

    libs = {
        "%VULKAN_SDK%/Lib/vulkan-1.lib"
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
        "YAML_CPP_STATIC_DEFINE",
        "VK_USE_PLATFORM_WIN32_KHR", --TODO add to windows filter
        "GLFW_EXPOSE_NATIVE_WIN32" --TODO add to windows filter
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
