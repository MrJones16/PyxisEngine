workspace "Pyxis"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Pixel-Game"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	IncludeDir = {}
	IncludeDir["spdlog"]	= "Pyxis/vendor/spdlog/include"
	IncludeDir["asio"]		= "Pyxis/vendor/asio/asio/include"
	IncludeDir["GLFW"]		= "Pyxis/vendor/GLFW/include"
	IncludeDir["GLAD"]		= "Pyxis/vendor/GLAD/include"
	IncludeDir["ImGui"]		= "Pyxis/vendor/ImGui"
	IncludeDir["glm"]		= "Pyxis/vendor/glm"
	IncludeDir["stb_image"] = "Pyxis/vendor/stb_image"
	IncludeDir["box2d"]		= "Pyxis/vendor/box2d/include"
	IncludeDir["tinyxml2"]	= "Pyxis/vendor/tinyxml2"
	IncludeDir["poly2tri"]	= "Pyxis/vendor/poly2tri"


	group "Dependencies"
		include "Pyxis/vendor/box2d"
		include "Pyxis/vendor/GLFW"
		include "Pyxis/vendor/GLAD"
		include "Pyxis/vendor/ImGui"
	group ""


	
project "Pyxis"
	location "Pyxis"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"


	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pxpch.h"
	pchsource "Pyxis/src/pxpch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/asio/asio/include/**.hpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/tinyxml2/**.cpp",
		"%{prj.name}/vendor/tinyxml2/**.h",
		"%{prj.name}/vendor/poly2tri/**.cpp",
		"%{prj.name}/vendor/poly2tri/**.h"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.asio}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.tinyxml2}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.poly2tri}"

	}

	links
	{
		"box2d",
		"GLFW",
		"GLAD",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"ASIO_STANDALONE",
			"_WIN32_WINNT=0x0A00",
			"PX_PLATFORM_WINDOWS",
			"PX_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		buildoptions
		{
			"-mwindows"
		}

	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PX_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_DIST"
		runtime "Release"
		optimize "on"


project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
	objdir ("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Pyxis/vendor/spdlog/include",
		"Pyxis/src",
		"Pyxis/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.asio}",
	}

	links
	{
		"Pyxis"
	}

	postbuildcommands 
	{
		"{COPYDIR} assets ../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/assets"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS"
		}

		buildoptions
		{
			"-mwindows"
		}


	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PX_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_DIST"
		runtime "Release"
		optimize "on"


		
project "Pyxis-Editor"
	location "Pyxis-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
	objdir ("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Pyxis/vendor/spdlog/include",
		"Pyxis/src",
		"Pyxis/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Pyxis"
	}

	postbuildcommands 
	{
		"{COPYDIR} assets ../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/assets"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PX_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_DIST"
		runtime "Release"
		optimize "on"

project "Pixel-Game"
	location "Pixel-Game"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
	objdir ("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Pyxis/vendor/spdlog/include",
		"Pyxis/src",
		"Pyxis/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.asio}",
	}

	links
	{
		"Pyxis"
	}

	postbuildcommands 
	{
		"{COPYDIR} assets ../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/assets"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PX_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_DIST"
		runtime "Release"
		optimize "on"


project "Pixel-Game-Server"
	location "Pixel-Game-Server"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
	objdir ("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Pyxis/vendor/spdlog/include",
		"Pyxis/src",
		"Pyxis/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.asio}",
	}

	links
	{
		"Pyxis",
		"Pixel-Game"
	}

	postbuildcommands 
	{
		"{COPYDIR} assets ../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/assets"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PX_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_DIST"
		runtime "Release"
		optimize "on"
		