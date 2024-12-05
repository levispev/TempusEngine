// Copyright Levi Spevakow (C) 2024

#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include <memory>


namespace Tempus {

	class TEMPUS_API Log
	{

	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		Log();
		~Log();

	private:

		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};


}


