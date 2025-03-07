project "Mule Editor"
	language "C++"
	kind "ConsoleApp"
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
        includes,
        "../Mule/include",
        "../Mule/include/mule",
        "../Submodules/IconFontCppHeaders",
        "src"
    }

    links {
        "ImGui",
        "glfw",
        "spdlog",
        "Mule",
        "yaml-cpp",
        "nativefiledialog",
        "ImGuizmo",
        libs,
        "coral.Native"
    }

    files {
        "src/**.h",
        "src/**.cpp"
    }

    filter {"configurations:Debug"}
        buildoptions {"/MDd"}
        links {
            debugLibs
        }
        
    filter {"configurations:Release"}
        buildoptions {"/MD"}
        links {
            releaseLibs
        }
        