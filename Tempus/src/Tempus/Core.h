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

// Core includes
#include <cstdint>
#include "Log.h"

// Helper macros
#define BIT(x) (1 << (x))
#define STRINGIFY(name) #name

// Assert macros
#define TPS_STATIC_ASSERT(...) static_assert(__VA_ARGS__)
#ifdef TPS_BUILD_DLL
	#define TPS_ASSERT(condition, ...) if(!(condition)) {TPS_CORE_CRITICAL(__VA_ARGS__);}
	#define TPS_ASSERT_WARN(condition, ...) if(!(condition)) {TPS_CORE_WARN(__VA_ARGS__);} 
#else
	#define TPS_ASSERT(condition, ...) if(!(condition)) {TPS_CRITICAL(__VA_ARGS__);}
	#define TPS_ASSERT_WARN(condition, ...) if(!(condition)) {TPS_WARN(__VA_ARGS__);} 
#endif

// Global constants
constexpr uint32_t MAX_ENTITIES = 5000;
constexpr uint8_t MAX_COMPONENTS = 32;

