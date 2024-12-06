// Copyright Levi Spevakow (C) 2024

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Tempus {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() 
	{
		// Set log print formatting
		spdlog::set_pattern("%^[%T] [%l] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("TEMPUS");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);

		TPS_CORE_INFO("Core log initialized!");
		TPS_INFO("Client log initialized!");

	}

	Tempus::Log::Log()
	{


	}

	Tempus::Log::~Log()
	{


	}

}

