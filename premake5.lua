workspace "Game Engine"
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
        dir .. "Submodules/bgfx/magma",
        "%VULKAN_SDK%/Include"
    } 

    vulkanLib = {
        "%VULKAN_SDK%/vulkan-1.lib"
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
        "YAML_CPP_STATIC_DEFINE"
    }
    
    -- Submodule
    group "Submodules"
        include "Submodules/imgui/premake5.lua"
        include "Submodules/glfw/premake5.lua"
        include "Submodules/spdlog/premake5.lua"
        include "Submodules/yaml/yaml.lua"
        include "Submodules/nativefiledialog/nativefiledialog.lua"
        include "Submodules/magma/magma.lua"
    group ""
    -- Projects
    include "Mule Editor/editor.lua"
    include "Mule/mule.lua"
