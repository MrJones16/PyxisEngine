project "GLAD"
	kind "StaticLib"
	language "C"
	architecture "x86_64"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    includedirs { "include/" }

	files { "src/glad.c", "include/glad/glad.h", "include/KHR/khrplatform.h" }
    
	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		defines
		{
			"_GLAD_X11"
		}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		defines 
		{ 
			"_GLAD_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"