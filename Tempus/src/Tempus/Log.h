// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include <memory>

#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_WHITE "\033[1;37m"
#define COLOR_RESET "\033[0m"

namespace Tempus {

#ifdef TPS_PLATFORM_WINDOWS
	template class TEMPUS_API std::shared_ptr<spdlog::logger>;
#endif

	class TEMPUS_API Log
	{

	public:

		Log();
		~Log();

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:

		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}

#ifdef TPS_BUILD_DLL
// Core log macros
#define TPS_CORE_TRACE(...)      ::Tempus::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TPS_CORE_INFO(...)       ::Tempus::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TPS_CORE_WARN(...)       ::Tempus::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TPS_CORE_ERROR(...)      ::Tempus::Log::GetCoreLogger()->error(__VA_ARGS__)
// Throws a runtime exception on use
#define TPS_CORE_CRITICAL(...)   do { \
                                     ::Tempus::Log::GetCoreLogger()->critical(__VA_ARGS__); \
                                     throw std::runtime_error(fmt::format(__VA_ARGS__)); \
                                 } while(0)
#else
#define TPS_CORE_TRACE(...)      static_assert(false, "TPS_CORE_TACE is not accessible from the client application.")
#define TPS_CORE_INFO(...)       static_assert(false, "TPS_CORE_INFO is not accessible from the client application.")
#define TPS_CORE_WARN(...)       static_assert(false, "TPS_CORE_WARN is not accessible from the client application.")
#define TPS_CORE_ERROR(...)      static_assert(false, "TPS_CORE_ERROR is not accessible from the client application.")
#define TPS_CORE_CRITICAL(...)   static_assert(false, "TPS_CORE_CRITICAL is not accessible from the client application.")
#endif

// Client log macros
#define TPS_TRACE(...)           ::Tempus::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TPS_INFO(...)            ::Tempus::Log::GetClientLogger()->info(__VA_ARGS__)
#define TPS_WARN(...)            ::Tempus::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TPS_ERROR(...)           ::Tempus::Log::GetClientLogger()->error(__VA_ARGS__)
// Throws a runtime exception on use
#define TPS_CRITICAL(...)        do { \
                                     ::Tempus::Log::GetClientLogger()->critical(__VA_ARGS__); \
                                     throw std::runtime_error(fmt::format(__VA_ARGS__)); \
                                 } while(0)
