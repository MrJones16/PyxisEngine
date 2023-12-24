#pragma once

#include "Core.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Pyxis
{

	class PYXIS_API Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger();
		static std::shared_ptr<spdlog::logger>& GetClientLogger();
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

//Core Log Macros
#define PX_CORE_TRACE(...)    ::Pyxis::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PX_CORE_INFO(...)     ::Pyxis::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PX_CORE_WARN(...)     ::Pyxis::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PX_CORE_ERROR(...)    ::Pyxis::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PX_CORE_CRITICAL(...) ::Pyxis::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client Log Macros
#define PX_TRACE(...)	      ::Pyxis::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PX_INFO(...)	      ::Pyxis::Log::GetClientLogger()->info(__VA_ARGS__)
#define PX_WARN(...)	      ::Pyxis::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PX_ERROR(...)         ::Pyxis::Log::GetClientLogger()->error(__VA_ARGS__)
#define PX_CRITICAL(...)      ::Pyxis::Log::GetClientLogger()->critical(__VA_ARGS__)


//if distribution build
 
//#define PX_CORE_TRACE  
//#define PX_CORE_INFO    
//#define PX_CORE_WARN    
//#define PX_CORE_ERROR   
//#define PX_CORE_CRITICAL
//
//#define PX_TRACE	 
//#define PX_INFO	 
//#define PX_WARN	 
//#define PX_ERROR    
//#define PX_CRITICAL