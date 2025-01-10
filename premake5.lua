workspace "Game Engine"
    configurations { "Debug", "Release" }
    architecture "x64"
    startproject "Mule Editor"

    dir = os.getcwd();

    includes = {
        dir .. "/Submodules/imgui",
        dir .. "/Submodules/glfw/include",
        dir .. "/Submodules/bgfx/include",
        dir .. "/Submodules/bx/include",
        dir .. "/Submodules/bimg/include",
        dir .. "/Submodules/bgfx/3rdparty",
        dir .. "/Submodules/spdlog/include",
        dir .. "/Submodules/stb",
        dir .. "/Submodules/entt/single_include",
        dir .. "/Submodules/glm",
        dir .. "/Submodules/yaml/include",
        dir .. "/Submodules/nativefiledialog/src/include",
        dir .. "/Submodules/tinygltf",
    } 

    debugLibDirs = {
        dir .. "/Submodules/bgfx/.build/win64_vs2022/bin"
    }

    releaseLibDirs = {
        dir .. "/Submodules/bgfx/.build/win64_vs2022/bin"
    }

    debugLibs = {
        "bgfxDebug.lib",
        "bxDebug.lib",
        "bimgDebug.lib",
        "bimg_encodeDebug.lib",
        "bimg_decodeDebug.lib",
    }

    releaseLibs = {
        "bgfxRelease.lib",
        "bxRelease.lib",
        "bimgRelease.lib",
        "bimg_encodeRelease.lib",
        "bimg_decodeRelease.lib",
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
        "YAML_CPP_STATIC_DEFINE"
    }
    
    -- Submodule
    include "Submodules/imgui/premake5.lua"
    include "Submodules/glfw/premake5.lua"
    include "Submodules/spdlog/premake5.lua"
    include "Submodules/yaml/yaml.lua"
    include "Submodules/nativefiledialog/nativefiledialog.lua"
    -- Projects
    include "Mule Editor/editor.lua"
    include "Mule/mule.lua"
