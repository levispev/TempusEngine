// Copyright Levi Spevakow (C) 2025

#include "Application.h"
#include <thread>
#include "Log.h"
#include <random>

#include "Window.h"
#include "Renderer.h"

#include "sdl/SDL_vulkan.h"
#include "Utils/FileUtils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"

namespace Tempus {

	Application::Application() : CurrentEvent(SDL_Event())
	{
		m_Window = new Window();
		m_Renderer = new Renderer();
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{

		Log::Init();

		// Changing working directory to project root.
		// @TODO in the future this will change if in a packaged build or if projects exist in a different location
		FileUtils::SetWorkingDirectory(FileUtils::GetExecutablePath());
		FileUtils::SetWorkingDirectory("../../../");

		// SDL Initialization
		if (!InitSDL()) 
		{
			return;
		}

		// Window creation
		if (!InitWindow()) 
		{
			return;
		}

		// Renderer creation
		if (!InitRenderer()) 
		{
			return;
		}

		while (!bShouldQuit) 
		{
			CoreUpdate();
		}

		Cleanup();

	}

	bool Application::InitWindow()
	{
		// Window creation
		if (!m_Window || !m_Window->Init("Sandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI))
		{
			TPS_CORE_CRITICAL("Failed to initialize window!");
			return false;
		}

		TPS_CORE_INFO("Window successfully created!");

		return true;

	}

	bool Application::InitRenderer()
	{

		// Renderer creation
		if (!m_Renderer || !m_Renderer->Init(m_Window))
		{
			TPS_CORE_CRITICAL("Failed to initialize renderer!");
			return false;
		}

		m_Renderer->SetRenderDrawColor(19, 16, 102, 255);

		TPS_CORE_INFO("Renderer successfully created!");

		return true;
	}

	bool Application::InitSDL()
	{
		SDL_SetMainReady();

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		{
			TPS_CORE_CRITICAL("Failed to initialize SDL: {0}", SDL_GetError());
			return false;
		}

		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

		SDL_version version;
		SDL_GetVersion(&version);
		TPS_CORE_INFO("Initialized SDL version {0}.{1}.{2}", version.major, version.minor, version.patch);

		if (SDL_Vulkan_LoadLibrary(nullptr)) 
		{
			TPS_CORE_CRITICAL("Failed to load Vulkan library: {0}", SDL_GetError());
			return false;
		}
		
		uint32_t instanceVersion = 0;

		if (vkEnumerateInstanceVersion(&instanceVersion) == VK_SUCCESS) 
		{
			TPS_CORE_INFO("Loaded Vulkan version: {0}.{1}.{2}", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
		}

		return true;
	}

	void Application::CoreUpdate()
	{

		while (SDL_PollEvent(&CurrentEvent)) 
		{
			ImGui_ImplSDL2_ProcessEvent(&CurrentEvent);

			if (CurrentEvent.type == SDL_QUIT)
			{
				bShouldQuit = true;
				return;
			}
		}

		Update();
		m_Renderer->Update();

	}

	void Application::Update()
	{
	}

	void Application::Cleanup()
	{

		if (m_Window) 
		{
			delete m_Window;
		}

		if (m_Renderer) 
		{
			delete m_Renderer;
		}

		SDL_Vulkan_UnloadLibrary();
		SDL_Quit();

		TPS_CORE_INFO("Application Cleaned");
	}

	void Application::SetRenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		m_Renderer->SetRenderDrawColor(r, g, b, a);
	}
}