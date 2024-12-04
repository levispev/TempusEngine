// Copyright Levi Spevakow (C) 2024

#pragma once

#ifdef TPS_PLATFORM_WINDOWS
	#ifdef TPS_BUILD_DLL
		#define TEMPUS_API __declspec(dllexport)
	#else
		#define TEMPUS_API __declspec(dllimport)
	#endif
#elif TPS_PLATFORM_MAC
	#ifdef TPS_BUILD_DLL
		#define TEMPUS_API __attribute__((visibility("default")))
	#else
    	#define TEMPUS_API
	#endif
#endif

