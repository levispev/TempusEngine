// Copyright Levi Spevakow (C) 2024

#pragma once

#include "Core.h"

#include "vulkan/vulkan.h"

namespace Tempus {

	class TEMPUS_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();

		VkInstance Instance = nullptr;

	};

}

