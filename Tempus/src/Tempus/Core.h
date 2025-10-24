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
#include <concepts>
#include "Log.h"

// Helper macros
#define BIT(x) (1 << (x))
#define STRINGIFY(name) #name

// Static Assert
#define TPS_STATIC_ASSERT(...) static_assert(__VA_ARGS__)

// Runtime assertion macros
#ifdef TPS_BUILD_DLL
	#define TPS_ASSERT(condition, ...) if(!(condition)) {TPS_CORE_CRITICAL(__VA_ARGS__);}
	#define TPS_ASSERT_WARN(condition, ...) if(!(condition)) {TPS_CORE_WARN(__VA_ARGS__);} 
#else
	#define TPS_ASSERT(condition, ...) if(!(condition)) {TPS_CRITICAL(__VA_ARGS__);}
	#define TPS_ASSERT_WARN(condition, ...) if(!(condition)) {TPS_WARN(__VA_ARGS__);} 
#endif

// Human readable debug name for classes at runtime
#ifndef TPS_DIST
    #define TPS_DEBUG_NAME(name) \
        public: \
        static constexpr const char* DebugName = name; \
        private:
#else
    #define TPS_DEBUG_NAME(name)
#endif

#define TPS_MACRO_JOIN(a, b) TPS_MACRO_JOIN_PRIVATE(a,b)
#define TPS_MACRO_JOIN_PRIVATE(a, b) a##b

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(Name) TPS_MACRO_JOIN(Name, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(Name) PREPROCESSOR_JOIN(Name, __LINE__)
#endif

// Macro for calling a void function once
#define TPS_CALL_ONCE(Func, ...) static int32_t ANONYMOUS_VARIABLE(UniqueOnce) = ((Func)(__VA_ARGS__), 1)

// Global constants
// Max entities per scene
constexpr uint32_t MAX_ENTITIES = 5000;
// Max components per entity
constexpr uint8_t MAX_COMPONENTS = 32;

namespace Tempus
{
	extern TEMPUS_API class Application* GApp;
}

namespace Tempus
{
    using ComponentId = uint8_t;
    class Component;

    // Concept for valid component
    // Constraint #1: Must derive from Component class
    // Constraint #2: Must implement valid ID using 'DECLARE_COMPONENT(ComponentName, ID)'
    template<typename T>
    concept ValidComponent = std::derived_from<T, Component> && requires {{T::GetId()} -> std::convertible_to<ComponentId>;};
}

