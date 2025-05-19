// Copyright Levi Spevakow (C) 2025

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "Utils/FileUtils.h"


namespace Tempus {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() 
	{
		// Set log print formatting
		spdlog::set_pattern("%^[%T] [%l] %n:%$ %v");

		// std::string cwd = FileUtils::GetExecutablePath();
		// std::string loggerName = "TEMPUS";
		// std::string logFile = "TempusLog.txt";
		//
		// std::vector<spdlog::sink_ptr> sinks
		// {
		// 	std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
		// 	std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile)
		// };
		//
		// s_CoreLogger = std::make_shared<spdlog::logger>(loggerName, begin(sinks), end(sinks));
		// s_CoreLogger->set_pattern("%^[%T] [%l] %n:%$ %v");
		// spdlog::register_logger(s_CoreLogger);
		// spdlog::set_default_logger(s_CoreLogger);
		// s_CoreLogger->set_level(spdlog::level::trace);
		// spdlog::flush_every(std::chrono::seconds(1));
		
		s_CoreLogger = spdlog::stdout_color_mt("TEMPUS");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);

		TPS_CORE_INFO("Core log initialized!");
		TPS_INFO("Client log initialized!");
	}
}

