#pragma once

#include <memory>

#ifdef PX_PLATFORM_WINDOWS
	#if PX_DYNAMIC_LINK
		#ifdef PX_BUILD_DLL
			#define PYXIS_API __declspec(dllexport)
		#else
			#define PYXIS_API __declspec(dllimport)
		#endif // !PX_BUILD_DLL
	#else
		#define PYXIS_API
	#endif
#else
	#error Pyxis only supports Windows at the moment
#endif // !PX_PLATFORM_WINDOWS

#ifdef PX_DEBUG
#define PX_ENABLE_ASSERTS
#define PX_PROFILING 0
#endif

#ifdef PX_RELEASE
#define PX_PROFILING 0
#endif

#ifdef PX_DIST
#define PX_PROFILING 0
#endif

#ifdef PX_ENABLE_ASSERTS
	#define PX_CORE_ASSERT(x, ...) {if(!(x)) {PX_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define PX_ASSERT(x, ...) {if(!(x)) {PX_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define PX_CORE_ASSERT(x,...)
	#define PX_ASSERT(x,...)
#endif

#define BIT(x) (1 << x)

#define PX_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#define STATISTICS 0

namespace Pyxis
{
	template <typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}



	template <typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}