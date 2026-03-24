// Copyright Levi Spevakow (C) 2025

#pragma once

// Helper macro for declaring bitwise operators for enum classes used as flags
#define ENUM_CLASS_FLAGS(Enum) \
inline constexpr Enum operator|(Enum a, Enum b) noexcept { using T = std::underlying_type_t<Enum>; return static_cast<Enum>(static_cast<T>(a) | static_cast<T>(b));} \
inline constexpr Enum operator&(Enum a, Enum b) noexcept {  using T = std::underlying_type_t<Enum>; return static_cast<Enum>(static_cast<T>(a) & static_cast<T>(b)); } \
inline constexpr Enum operator^(Enum a, Enum b) noexcept { using T = std::underlying_type_t<Enum>; return static_cast<Enum>(static_cast<T>(a) ^ static_cast<T>(b)); } \
inline constexpr Enum operator~(Enum a) noexcept { using T = std::underlying_type_t<Enum>; return static_cast<Enum>(~static_cast<T>(a)); } \
inline Enum& operator|=(Enum& a, Enum b) noexcept { a = a | b; return a; } \
inline Enum& operator&=(Enum& a, Enum b) noexcept { a = a & b; return a; } \
inline Enum& operator^=(Enum& a, Enum b) noexcept { a = a ^ b; return a; }

template<typename Enum>
constexpr bool EnumCheckFlag(Enum value, Enum flag) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return (static_cast<T>(value) & static_cast<T>(flag)) != 0;
}

template<typename Enum>
constexpr bool EnumSetFlags(Enum value, Enum flags) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<T>(value) | static_cast<T>(flags));
}

template<typename Enum>
constexpr Enum EnumClearFlags(Enum value, Enum flag) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<T>(value) & ~static_cast<T>(flag));
}

// @TODO Should be moved somewhere else, or this file should be renamed
#ifndef TPS_DIST
#define DECLARE_ENUM_NAMES(EnumType, ...) \
    inline const char* GetEnumName(EnumType value) \
    { \
        static const char* names[] = { __VA_ARGS__ }; \
        int v = static_cast<int>(value); \
        if (v == 0) return "None"; \
        int index = 0; \
        while (v > 1) { v >>= 1; index++; } \
        if (index >= 0 && index < (int)(sizeof(names) / sizeof(names[0]))) \
        { \
            return names[index]; \
        } \
        return "Unknown"; \
    }
#else
#define DECLARE_ENUM_NAMES(EnumType, ...)
#endif