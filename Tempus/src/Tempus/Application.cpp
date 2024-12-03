// Copyright Levi Spevakow (C) 2024

#include "Application.h"
#include <iostream>


namespace Tempus {

	Tempus::Application::Application()
	{
		
	}

	Tempus::Application::~Application()
	{
	}

	void Application::Run()
	{
		std::cout << "Hello World!" << std::endl;

		m_Window = glfwCreateWindow(1920, 1080, "Sandbox", glfwGetPrimaryMonitor(), nullptr);

		std::cin.get();
	}


}

