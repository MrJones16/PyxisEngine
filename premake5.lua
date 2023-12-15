workspace "Pyxis"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Sandbox"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	IncludeDir = {}
	IncludeDir["GLFW"] = "Pyxis/vendor/GLFW/include"
	IncludeDir["GLAD"] = "Pyxis/vendor/GLAD/include"
	IncludeDir["ImGui"] = "Pyxis/vendor/ImGui"

	group "Dependencies"
		include "Pyxis/vendor/GLFW"
		include "Pyxis/vendor/GLAD"
		include "Pyxis/vendor/ImGui"
	group ""



project "Pyxis"
	location "Pyxis"
	kind "SharedLib"
	language "C++"
	staticruntime "off"


	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pxpch.h"
	pchsource "Pyxis/src/pxpch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS",
			"PX_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox");
		}

	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "PX_Release"
		runtime "Release"
		symbols "On"

	filter "configurations:Dist"
		defines "PX_Dist"
		runtime "Release"
		symbols "On"


project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

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
		"Pyxis/src"
	}

	links
	{
		"Pyxis"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "PX_Release"
		runtime "Release"
		symbols "On"

	filter "configurations:Dist"
		defines "PX_Dist"
		runtime "Release"
		symbols "On"
