// Copyright Levi Spevakow (C) 2024

#pragma once

#include "Core.h"

#include "vulkan/vulkan.h"
#include "glfw3.h"


namespace Tempus {

	class TEMPUS_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();

		VkInstance m_Instance = nullptr;

		GLFWwindow* m_Window;

	};

}

