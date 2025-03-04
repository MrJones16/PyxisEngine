#pragma once

#include <memory>
#include <chrono>
#include <functional>
#include <glm/glm.hpp>
//#include <fmt/ostream.h>

//template <> struct fmt::formatter<glm::mat4> : ostream_formatter {};

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

	template <typename T>
	using WeakRef = std::weak_ptr<T>;

	template<typename T>
	constexpr WeakRef<T> CreateWeakRef(const Ref<T>& ref)
	{
		return WeakRef<T>(ref);
	}



	template<typename Fn>
	class Timer
	{
	public:
		Timer(const char* name, Fn&& func)
			: m_Name(name), m_Stopped(false), m_Func(func)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~Timer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();
			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			m_Stopped = true;
			float duration = (end - start) * 0.001f;

			m_Func({ m_Name, duration });
			//std::cout << m_Name << ": Duration: " << duration << "milliseconds" << std::endl;
		}
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		bool m_Stopped;
		Fn m_Func;
	};

#if PX_PROFILING
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfilingPanel->m_ProfileResults.push_back(profileResult);})
#else
#define PROFILE_SCOPE(name)
#endif

	/*template<typename T>
	class Callback
	{
	public:

		std::function<T> m_CallBackFunction = nullptr;
		Callback()
		{

		}

		Callback(void* func)
		{
			m_CallBackFunction = func;
		}

		void operator () ()
		{
			m_CallBackFunction();
		}

	};*/

}

namespace glm
{
	//override < operator for glm ivec2
	inline bool operator<(const glm::ivec2& lhs, const glm::ivec2& rhs)
	{
		if (lhs.x < rhs.x) return true;
		if (lhs.x > rhs.x) return false;
		return lhs.y < rhs.y;
	}

	//override == operator for glm ivec2
	inline bool operator==(const glm::ivec2& lhs, const glm::ivec2& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
}