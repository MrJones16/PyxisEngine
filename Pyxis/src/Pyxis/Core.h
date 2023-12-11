#pragma once

#ifdef PX_PLATFORM_WINDOWS
	#ifdef PX_BUILD_DLL
		#define PYXIS_API __declspec(dllexport)
	#else
		#define PYXIS_API __declspec(dllimport)
	#endif // !PX_BUILD_DLL
#else
	#error Pyxis only supprts Windows at the moment
#endif // !PX_PLATFORM_WINDOWS

#define BIT(x) (1 << x)