// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Log.h"

#define TPS_ASSERT(condition, message) if(!(condition)) {TPS_CORE_CRITICAL(message);};
#define TPS_ASSERT_ERROR(condition, message) if(!(condition)) {TPS_CORE_ERROR(message);};
#define TPS_ASSERT_WARN(condition, message) if(!(condition)) {TPS_CORE_WARN(message);};
