project "Mule Editor"
	language "C++"
	kind "ConsoleApp"
	location ""
	cppdialect "C++20"

    buildoptions {"/MP"}
    buildoptions {"/Zc:preprocessor"}
    buildoptions {"/Zc:__cplusplus"}
    buildoptions {"/utf-8"} -- Needed for spdlog to compile

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
        "opengl32.lib",
        "Mule",
        "yaml-cpp",
        "nativefiledialog"
    }

    files {
        "src/**.h",
        "src/**.cpp"
    }

    filter {"configurations:Debug"}
        buildoptions {"/MTd"}
        libdirs {
            debugLibDirs
        }
        links {
            debugLibs
        }
        defines {
            "BX_CONFIG_DEBUG"
        }
        
    filter {"configurations:Release"}
        buildoptions {"/MT"}
        libdirs {
            releaseLibDirs
        }
        links {
            releaseLibs
        }
        