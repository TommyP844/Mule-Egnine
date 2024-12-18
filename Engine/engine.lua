project "Engine"
	language "C++"
	kind "StaticLib"
	location ""
	cppdialect "C++20"

	linkoptions { '/NODEFAULTLIB:"libcpmtd.lib"' }
	buildoptions {'/MP'}

    includedirs {
        "include",
        includes
    }

    links {
        "ImGui",
        "glfw",
        "spdlog"
    }

    files {
        "include/**.h",
        "src/**.cpp"
    }

    filter {"configurations:Debug"}
        libdirs {
            debugLibDirs
        }
        links {
            debugLibs
        }
        
    filter {"configurations:Release"}
        libdirs {
            releaseLibDirs
        }
        links {
            releaseLibs
        }