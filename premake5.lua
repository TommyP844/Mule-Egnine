workspace "Game Engine"
    configurations { "Debug", "Release" }
    architecture "x64"
    startproject "Editor"

    dir = os.getcwd();

    includes = {
        dir .. "/Submodules/imgui",
        dir .. "/Submodules/glfw/include",
        dir .. "/Submodules/bgfx/include",
        dir .. "/Submodules/bgfx/3rdparty",
        dir .. "/Submodules/spdlog/include"
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
    
    -- Submodules
    include "Submodules/imgui/premake5.lua"
    include "Submodules/glfw/premake5.lua"
    include "Submodules/spdlog/premake5.lua"
    -- Projects
    include "Editor/Editor.lua"
    include "Engine/Engine.lua"
