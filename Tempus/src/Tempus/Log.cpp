// Copyright Levi Spevakow (C) 2025

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "Utils/FileUtils.h"
#include <filesystem>

namespace Tempus {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() 
	{
		// Create sinks for both console and rotating file
		std::vector<spdlog::sink_ptr> sinks;
		
		// Console sink with colors
		sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		
		// Get logs directory path (creates it if it doesn't exist)
		auto logFilePath = FileUtils::LogsDir() / "TempusLog.txt";
		
		// Rotating file sink. Max 5MB per file, keep 3 files
		auto rotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			logFilePath.string(), 1024 * 1024 * 5, 3, /*rotate_on_open=*/true);
		
		sinks.push_back(rotatingSink);
		
		// Set pattern for all sinks
		for (auto& sink : sinks)
		{
			sink->set_pattern("%^[%T] [%l] %n:%$ %v");
		}
		
		// Create core logger with both sinks
		s_CoreLogger = std::make_shared<spdlog::logger>("TEMPUS", begin(sinks), end(sinks));
		s_CoreLogger->set_level(spdlog::level::trace);
		spdlog::register_logger(s_CoreLogger);

		// Create client logger with both sinks
		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(sinks), end(sinks));
		s_ClientLogger->set_level(spdlog::level::trace);
		spdlog::register_logger(s_ClientLogger);

		// Flush logs every 3 seconds to ensure they're written to disk
		spdlog::flush_every(std::chrono::seconds(3));

		TPS_CORE_INFO("Core log initialized!");
		TPS_INFO("Client log initialized!");
		TPS_CORE_INFO("Log file location: {0}", logFilePath.string());
	}
}
