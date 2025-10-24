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
#include "Utils/Time.h"

namespace Tempus
{
	Application* GApp = nullptr;
}

Tempus::Application::Application() : CurrentEvent(SDL_Event()), AppName("Application Name")
{
	GApp = this;
	m_Window = new Window();
	m_Renderer = new Renderer();
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
	//SCENE_MANAGER->DoTestScene();

	AppStart();

	while (!bShouldQuit) 
	{
		CoreUpdate();
	}

	Cleanup();
	
}

void Tempus::Application::RegisterUpdateable(IUpdateable* updatable)
{
	m_Updateables.insert(updatable);
}

void Tempus::Application::UnregisterUpdateable(IUpdateable* updatable)
{
	m_Updateables.erase(updatable);
}

uint32_t Tempus::Application::GetUpdateableCount() const
{
	return m_Updateables.size();
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
	if (!m_Renderer || !m_Renderer->Init(m_Window))
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

void Tempus::Application::CoreUpdate()
{
	Time::CalculateDeltaTime();
	float dt = Time::GetDeltaTime() * Time::GetTimeScale();
	EventUpdate();

	for (IUpdateable* updatable : m_Updateables)
	{
		updatable->OnUpdate(Time::GetDeltaTime() * Time::GetTimeScale());
	}
	
	Update();
	m_Renderer->Update();
	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void Tempus::Application::EventUpdate()
{
	SDL_PollEvent(&CurrentEvent);

	ImGui_ImplSDL2_ProcessEvent(&CurrentEvent);

	if (CurrentEvent.type == SDL_QUIT)
	{
		bShouldQuit = true;
		return;
	}
	else 
	{
		// @TODO Temporarily manually managing input here
		ProcessInput(CurrentEvent);
		EVENT_DISPATCHER->Propagate(CurrentEvent);
	}
}

void Tempus::Application::ProcessInput(SDL_Event event)
{
	if (event.type == SDL_KEYDOWN) // Temporary input handling 
	{
		char key = (char)event.key.keysym.sym;
		//TPS_CORE_TRACE("DOWN: {0}", (int)event.key.keysym.scancode);
		if(m_InputMap.contains(event.key.keysym.scancode))
		{
			m_InputBits.set(m_InputMap[event.key.keysym.scancode]);	
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
	
	UpdateEditorCamera();
}

void Tempus::Application::UpdateEditorCamera()
{
	Scene* activeScene = SCENE_MANAGER->GetActiveScene();
	if (!activeScene)
	{
		return;
	}

	CameraComponent* camComp = activeScene->GetComponent<CameraComponent>(0);
	TransformComponent* transComp = activeScene->GetComponent<TransformComponent>(0);

	if (camComp && transComp)
	{
		// Forward / Back Movement
		transComp->Position += transComp->GetForwardVector() * (m_InputBits.test(0) * (Time::GetDeltaTime() * 10.0f));
		transComp->Position -= transComp->GetForwardVector() * (m_InputBits.test(2) * (Time::GetDeltaTime() * 10.0f));
		// Right / Left Movement
		transComp->Position -= transComp->GetRightVector() * (m_InputBits.test(1) * (Time::GetDeltaTime() * 10.0f));
		transComp->Position += transComp->GetRightVector() * (m_InputBits.test(3) * (Time::GetDeltaTime() * 10.0f));
		// Up / Down Movement
		transComp->Position.z -= m_InputBits.test(4) * (Time::GetDeltaTime() * 10.0f);
		transComp->Position.z += m_InputBits.test(5) * (Time::GetDeltaTime() * 10.0f);
		// Pitch rotation
		transComp->Rotation.x += m_InputBits.test(6) * (Time::GetDeltaTime() * 100.0f);
		transComp->Rotation.x -= m_InputBits.test(7) * (Time::GetDeltaTime() * 100.0f);
		transComp->Rotation.x = glm::clamp(transComp->Rotation.x, -89.0f, 89.0f);
		// Yaw Rotation
		transComp->Rotation.y += m_InputBits.test(8) * (Time::GetDeltaTime() * 100.0f);
		transComp->Rotation.y -= m_InputBits.test(9) * (Time::GetDeltaTime() * 100.0f);
		if (transComp->Rotation.y > 360.0f || transComp->Rotation.y < -360.0f)
		{
			transComp->Rotation.y = 0.0f;
		}
	}
}

void Tempus::Application::AppStart()
{
}

void Tempus::Application::Update()
{
}

void Tempus::Application::Cleanup()
{
	delete m_Renderer;
	delete m_Window;
	
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();

	TPS_CORE_INFO("Application Cleaned");
}

void Tempus::Application::SetRenderClearColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	m_Renderer->SetClearColor(r, g, b, a);
}
