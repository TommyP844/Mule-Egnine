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
        "%VULKAN_SDK%/Include"
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
    group ""
    -- Projects
    include "Mule Editor/editor.lua"
    include "Mule/mule.lua"
