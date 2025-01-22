// Copyright Levi Spevakow (C) 2025

#pragma once

#ifdef TPS_PLATFORM_WINDOWS
	#ifdef TPS_BUILD_DLL
		#define TEMPUS_API __declspec(dllexport)
	#else
		#define TEMPUS_API __declspec(dllimport)
	#endif

	#define VK_USE_PLATFORM_WIN32_KHR
	#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	#define DESIRED_VK_LAYER "VK_LAYER_KHRONOS_validation"

#elif TPS_PLATFORM_MAC
	#ifdef TPS_BUILD_DLL
		#define TEMPUS_API __attribute__((visibility("default")))
	#else
    	#define TEMPUS_API
	#endif

	#define VK_USE_PLATFORM_MACOS_MVK
	#define PLATFORM_SURFACE_EXTENSION_NAME VK_MVK_MACOS_SURFACE_EXTENSION_NAME
	#define DESIRED_VK_LAYER "MoltenVK"
	
#else
#error Tempus only supports Windows and Mac!
#endif

// Helper macros
#define BIT(x) (1 << x)
#define ENUM_TO_STRING(name) #name