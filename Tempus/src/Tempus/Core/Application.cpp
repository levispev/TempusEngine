// Copyright Levi Spevakow (C) 2025

#include "Application.h"
#include <thread>
#include "Log.h"
#include <random>
#include <iostream>

#include "Window.h"
#include "Renderer.h"
#include "Components/CameraComponent.h"
#include "Events/EventDispatcher.h"

#include "sdl/SDL_vulkan.h"
#include "Utils/FileUtils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"

#include "Managers/SceneManager.h"
#include "Entity/Entity.h"
#include "Components/TransformComponent.h"
#include "Utils/Profiling.h"
#include "Utils/Time.h"

namespace Tempus
{
	TEMPUS_API Application* GApp = nullptr;
}

Tempus::Application::Application() : CurrentEvent(SDL_Event()), AppName("Application Name")
{
	GApp = this;
	m_Window = std::make_unique<Window>();
	m_Renderer = std::make_unique<Renderer>();

	// Temporarily hard coding these values.
	// Will be read from a config system once set up
	m_MouseSensitivity = 0.25f;
#if TPS_PLATFORM_WINDOWS
	m_CatchMouseButton = SDL_BUTTON_RIGHT;
#elif TPS_PLATFORM_MAC
	m_CatchMouseButton = SDL_BUTTON_LEFT;
#endif

}

Tempus::Application::~Application()
{
	GApp = nullptr;
}

void Tempus::Application::Run()
{
#pragma region TEMPUS_LOGO
	std::cout << COLOR_BLUE <<
R"(
	  888888888888  88888888888  88b           d88  88888888ba   88        88   ad88888ba
	      88       88           888b         d888  88      "8b  88        88  d8"     "8b
	     88       88           88`8b       d8'88  88      ,8P  88        88  Y8,         
	    88       88aaaaa      88 `8b     d8' 88  88aaaaaa8P'  88        88  `Y8aaaaa,    
	   88       88"""""      88  `8b   d8'  88  88""""""'    88        88    `"""""8b,   
	  88       88           88   `8b d8'   88  88           88        88          `8b    
	 88       88           88    `888'    88  88           Y8a.    .a8P  Y8a     a8P     
	88       88888888888  88     `8'     88  88            `"Y8888Y"'    "Y88888P"       
)"
#pragma endregion
	
	<< '\n' << COLOR_RESET << std::flush;

	Log::Init();

	// Changing working directory to project root.
	// @TODO in the future this will change if in a packaged build or if projects exist in a different location
	FileUtils::SetWorkingDirectory(FileUtils::GetExecutablePath());
	FileUtils::SetWorkingDirectory("../../../");

	InitSDL();

	InitManagers();

	InitWindow();

	InitRenderer();

	// Printing registered components
	auto components = TPS_Private::ComponentRegistry::GetRegisteredComponents();
	std::stringstream ss;
	ss << "Registered Components: \n";
	for(auto component : components)
	{
		ss << " - " << component.name << '[' << static_cast<uint32_t>(component.id) << ']' << "\n";
	}
	TPS_CORE_INFO(ss.str());
	
	SCENE_MANAGER->CreateScene("Test Scene");

	AppStart();

	while (!bShouldQuit) 
	{
		CoreUpdate();
	}

	Cleanup();
	
}

void Tempus::Application::InitWindow()
{
	// Window creation
	if (!m_Window || !m_Window->Init(AppName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE))
	{
		TPS_CORE_CRITICAL("Failed to initialize window!");
	}

	m_Window->SetIcon("Logo.png");

	TPS_CORE_INFO("Window successfully created!");
}

void Tempus::Application::InitRenderer()
{
	// Renderer creation
	if (!m_Renderer || !m_Renderer->Init(m_Window.get()))
	{
		TPS_CORE_CRITICAL("Failed to initialize renderer!");
	}

	m_Renderer->SetClearColor(19, 16, 102, 255);

	TPS_CORE_INFO("Renderer successfully created!");
}

void Tempus::Application::InitSDL()
{
	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
	{
		TPS_CORE_CRITICAL("Failed to initialize SDL: {0}", SDL_GetError());
	}

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

	SDL_version version;
	SDL_GetVersion(&version);
	TPS_CORE_INFO("Initialized SDL version {0}.{1}.{2}", version.major, version.minor, version.patch);

	if (SDL_Vulkan_LoadLibrary(nullptr)) 
	{
		TPS_CORE_CRITICAL("Failed to load Vulkan library: {0}", SDL_GetError());
	}
	
	uint32_t instanceVersion = 0;

	if (vkEnumerateInstanceVersion(&instanceVersion) == VK_SUCCESS) 
	{
		TPS_CORE_INFO("Loaded Vulkan version: {0}.{1}.{2}", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
	}
}

void Tempus::Application::InitManagers()
{
	// All managers initialized here
	CreateManager<SceneManager>();
}

void Tempus::Application::CoreUpdate()
{
	Time::CalculateDeltaTime();
	EventUpdate();
	ManagerUpdate();
	AppUpdate();
	m_Renderer->Update(Time::GetUnscaledDeltaTime());
}

void Tempus::Application::ManagerUpdate()
{
	TPS_SCOPED_TIMER();
	for (auto& manager : m_Managers)
	{
		if (manager.second->IsUpdating())
		{
			manager.second->OnUpdate(Time::GetDeltaTime());
		}
	}
}

void Tempus::Application::EventUpdate()
{
	m_MouseDeltaX = 0;
	m_MouseDeltaY = 0;
	
	while (SDL_PollEvent(&CurrentEvent))
	{
		ImGui_ImplSDL2_ProcessEvent(&CurrentEvent);

		if (CurrentEvent.type == SDL_QUIT)
		{
			bShouldQuit = true;
		}
		else
		{
			// @TODO Temporarily manually managing input here
			ProcessInput(CurrentEvent);
			EVENT_DISPATCHER->Propagate(CurrentEvent);
		}
	}

	// If the mouse is captured, then update the editor camera
	if (SDL_GetRelativeMouseMode() == SDL_TRUE)
	{
		UpdateEditorCamera();
	}
}

void Tempus::Application::ProcessInput(SDL_Event event)
{
	ImGuiIO& io = ImGui::GetIO();

	if (event.type == SDL_KEYDOWN) // Temporary input handling 
	{
		char key = (char)event.key.keysym.sym;
		//TPS_CORE_TRACE("DOWN: {0}", (int)event.key.keysym.scancode);
		if(m_InputMap.contains(event.key.keysym.scancode))
		{
			m_InputBits.set(m_InputMap[event.key.keysym.scancode]);	
		}

		// Entity focus bound on F
		if(event.key.keysym.scancode == SDL_SCANCODE_F)
		{
			if(m_Renderer)
			{
				m_Renderer->FocusSelectedEntity();
			}
		}

		if (event.key.keysym.scancode == SDL_SCANCODE_F11)
		{
			m_Window->SetFullscreen(!m_Window->IsFullscreen());
		}
		
	}
	else if (event.type == SDL_KEYUP)
	{
		char key = (char)event.key.keysym.sym;
		//TPS_CORE_TRACE("UP: {0}", (int)event.key.keysym.scancode);
		if(m_InputMap.contains(event.key.keysym.scancode))
		{
			m_InputBits.reset(m_InputMap[event.key.keysym.scancode]);	
		}
	}
	else if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		// Ignore if hovering over ImGui element
		if(io.WantCaptureMouse)
		{
			return;
		}
		if (event.button.button == m_CatchMouseButton)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			SDL_SetWindowMouseGrab(m_Window->GetNativeWindow(), SDL_TRUE);
			// Store current x & y for warp back on release
			m_SavedMouseX = event.motion.x;
			m_SavedMouseY = event.motion.y;
		}
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		// Ignore if hovering over ImGui element
		if(io.WantCaptureMouse)
		{
			return;
		}
		if (event.button.button == m_CatchMouseButton)
		{
			if (SDL_GetRelativeMouseMode() == SDL_TRUE)
			{
				// Warp cursor back to location on mouse catch
				SDL_WarpMouseInWindow(m_Window->GetNativeWindow(), m_SavedMouseX, m_SavedMouseY);
				m_LastMouseX = m_SavedMouseX;
				m_LastMouseY = m_SavedMouseY;
			}
			SDL_SetRelativeMouseMode(SDL_FALSE);
			SDL_SetWindowMouseGrab(m_Window->GetNativeWindow(), SDL_FALSE);
		}
	}
	else if (event.type == SDL_MOUSEMOTION)
	{
		ProcessMouseMovement(event);
	}

}

void Tempus::Application::ProcessMouseMovement(SDL_Event event)
{
	m_MouseDeltaX += event.motion.xrel;
	m_MouseDeltaY -= event.motion.yrel;

	m_LastMouseX = event.motion.x;
	m_LastMouseY = event.motion.y;
}

void Tempus::Application::UpdateEditorCamera()
{
	TPS_SCOPED_TIMER();
	
	Scene* activeScene = SCENE_MANAGER->GetActiveScene();
	if (!activeScene)
	{
		return;
	}

	if (!activeScene->HasEntity(0))
	{
		return;
	}

	CameraComponent* camComp = activeScene->GetComponent<CameraComponent>(0);
	TransformComponent* transComp = activeScene->GetComponent<TransformComponent>(0);

	// Cam speed up
	if(m_InputBits.test(6))
	{
		m_EditorCamSpeed = 1500.0f;
	}
	else if (m_InputBits.test(7))
	{
		// Cam slow down
		m_EditorCamSpeed = 30.0f;
	}
	else
	{
		// Base camera speed
		m_EditorCamSpeed = 500.0f;
	}
	
	if (camComp && transComp)
	{
		// Forward / Back Movement
		transComp->Position += transComp->GetForwardVector() * (m_InputBits.test(0) * (Time::GetUnscaledDeltaTime() * m_EditorCamSpeed));
		transComp->Position -= transComp->GetForwardVector() * (m_InputBits.test(2) * (Time::GetUnscaledDeltaTime() * m_EditorCamSpeed));
		// Right / Left Movement
		transComp->Position -= transComp->GetRightVector() * (m_InputBits.test(1) * (Time::GetUnscaledDeltaTime() * m_EditorCamSpeed));
		transComp->Position += transComp->GetRightVector() * (m_InputBits.test(3) * (Time::GetUnscaledDeltaTime() * m_EditorCamSpeed));
		// Up / Down Movement
		transComp->Position.z -= m_InputBits.test(4) * (Time::GetUnscaledDeltaTime() * m_EditorCamSpeed);
		transComp->Position.z += m_InputBits.test(5) * (Time::GetUnscaledDeltaTime() * m_EditorCamSpeed);
		// Pitch rotation
		transComp->Rotation.x += m_MouseDeltaY * m_MouseSensitivity;
		transComp->Rotation.x = glm::clamp(transComp->Rotation.x, -89.0f, 89.0f);
		// Yaw Rotation
		transComp->Rotation.y += m_MouseDeltaX * m_MouseSensitivity;
		if (transComp->Rotation.y > 360.0f || transComp->Rotation.y < -360.0f)
		{
			transComp->Rotation.y = 0.0f;
		}
	}
}

void Tempus::Application::AppStart()
{
}

void Tempus::Application::AppUpdate()
{
}

void Tempus::Application::Cleanup()
{
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();

	TPS_CORE_INFO("Application Cleaned");
}

void Tempus::Application::SetRenderClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	m_Renderer->SetClearColor(r, g, b, a);
}
