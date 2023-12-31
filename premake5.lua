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
	IncludeDir["glm"] = "Pyxis/vendor/glm"
	IncludeDir["stb_image"] = "Pyxis/vendor/stb_image"


	group "Dependencies"
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
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"

	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PX_PLATFORM_WINDOWS",
			"PX_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "PX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PX_Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_Dist"
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
		"%{IncludeDir.glm}"
	}

	links
	{
		"Pyxis"
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
		defines "PX_Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PX_Dist"
		runtime "Release"
		optimize "on"
