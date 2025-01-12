project "Mule"
	language "C++"
	kind "StaticLib"
	location ""
	cppdialect "C++20"

    buildoptions {"/MP"}
    buildoptions {"/Zc:preprocessor"}
    buildoptions {"/Zc:__cplusplus"}
    buildoptions {"/utf-8"} -- Needed for spdlog to compile

    includedirs {
        "include/imguiImpl",
        "include/mule",
        includes
    }

    links {
        "ImGui",
        "glfw",
        "spdlog",
        "opengl32.lib",
        "yaml-cpp",
        "magma",
        vulkanLib
    }

    files {
        "include/**.h",
        "include/**.inl",
        "src/**.cpp"
    }

    filter {"configurations:Debug"}
        buildoptions {"/MTd"}
        defines {
            "BX_CONFIG_DEBUG"
        }
        
    filter {"configurations:Release"}
        buildoptions {"/MT"}