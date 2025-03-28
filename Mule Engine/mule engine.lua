project "Mule Engine"
	language "C++"
	kind "StaticLib"
	location ""
	cppdialect "C++20"
    architecture "x64"

    buildoptions {"/MP"}
    buildoptions {"/Zc:preprocessor"}
    buildoptions {"/Zc:__cplusplus"}
    buildoptions {"/utf-8"} -- Needed for spdlog to compile

    defines {
        "GLFW_INCLUDE_VULKAN"
    }

    includedirs {
        "include/imguiImpl",
        "include/mule",
        includes
    }

    links {
        "ImGui",
        "glfw",
        "spdlog",
        "yaml-cpp",
        libs,
        "Coral.Native"
    }

    files {
        "include/**.h",
        "include/**.inl",
        "src/**.cpp"
    }

    filter {"configurations:Debug"}
        links {
            debugLibs
        }
        
    filter {"configurations:Release"}
        links {
            releaseLibs
        }