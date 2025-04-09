workspace "Mule"
    configurations { "Debug", "Release" }
    startproject "Mule Editor"

    dir = os.getcwd();

    filter "configurations:Debug"
        staticruntime "Off"
        runtime "Debug"

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
        "%VULKAN_SDK%/Include",
        dir .. "/Submodules/Coral/Coral.Native/Include",
        dir .. "/Submodules/JoltPhysics"
    }

    debugLibs = {
        "%VULKAN_SDK%/Lib/shadercd.lib",
        "%VULKAN_SDK%/Lib/shaderc_combinedd.lib",
        "%VULKAN_SDK%/Lib/shaderc_utild.lib",
        dir .. "/Submodules/Assimp/lib/Debug/assimp-vc143-mtd.lib",
        dir .. "/Submodules/Assimp/lib/Debug/dracod.lib",
        dir .. "/Submodules/Assimp/lib/Debug/dracod.lib",
        dir .. "/Submodules/Assimp/contrib/zlib/Debug/zlibstaticd.lib",
        dir .. "/Submodules/JoltPhysics/Build/VS2022_CL/Debug/Jolt.lib"
    }

    releaseLibs = {
        "%VULKAN_SDK%/Lib/shaderc.lib",
        "%VULKAN_SDK%/Lib/shaderc_combined.lib",
        "%VULKAN_SDK%/Lib/shaderc_util.lib",
        dir .. "/Submodules/Assimp/lib/Release/assimp-vc143-mt.lib",
        dir .. "/Submodules/Assimp/lib/Release/draco.lib",
        dir .. "/Submodules/JoltPhysics/Build/VS2022_CL/Release/Jolt.lib"
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
        "ASSIMP_BUILD_NO_EXPORT",
        "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
        "_HAS_EXCEPTIONS=0",
        "JPH_DEBUG_RENDERER",
        "JPH_PROFILE_ENABLED",
        "JPH_OBJECT_STREAM",
        "JPH_USE_AVX2",
        "JPH_USE_AVX",
        "JPH_USE_SSE4_1",
        "JPH_USE_SSE4_2",
        "JPH_USE_LZCNT",
        "JPH_USE_TZCNT",
        "JPH_USE_F16C",
        "JPH_USE_FMADD",
        "VK_NO_PROTOTYPES",
        "NOMINMAX"
    }

    -- Coral
    postbuildcommands {
		'{COPYFILE} "%{wks.location}Submodules/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}Submodules/Coral/Build/%{cfg.targetdir}"',
	}
    
    -- Submodule
    group "Submodules"
        include "Submodules/imgui/premake5.lua"
        include "Submodules/glfw/premake5.lua"
        include "Submodules/spdlog/premake5.lua"
        include "Submodules/yaml/yaml.lua"
        include "Submodules/nativefiledialog/nativefiledialog.lua"
        include "Submodules/ImGuizmo/imguizmo.lua"
        include "Submodules/Coral/Coral.Managed/premake5.lua"
        include "Submodules/Coral/Coral.Native/premake5.lua"
    group ""
    -- Projects
    include "Mule Editor/editor.lua"
    include "Mule Engine/mule engine.lua"
    include "MuleScriptEngine/MuleScriptEngine.lua"
