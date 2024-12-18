project "Editor"
	language "C++"
	kind "StaticLib"
	location ""
	cppdialect "C++20"

	linkoptions { '/NODEFAULTLIB:"libcpmtd.lib"' }
	buildoptions {'/MP'}

    includedirs {
        includes
    }

    links {
        "ImGui",
        "glfw",
        "spdlog"
    }

    files {
        "src/**.h",
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
        