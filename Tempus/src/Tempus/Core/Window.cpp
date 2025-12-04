// Copyright Levi Spevakow (C) 2025

#include "Window.h"

#include "Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"


Tempus::Window::Window()
{
}

Tempus::Window::~Window()
{
	Cleanup();
}

bool Tempus::Window::Init(const char* title,
	int x, int y, int w,
	int h, Uint32 flags)
{
	std::string windowTitle = std::format("{} ({})", title, TPS_CONFIG_NAME);

	m_Width = static_cast<uint32_t>(w);
	m_Height = static_cast<uint32_t>(h);
	m_Window = SDL_CreateWindow(windowTitle.c_str(), w, h, flags);

	return m_Window;
}

SDL_Window* Tempus::Window::GetNativeWindow() const
{
	return m_Window;
}

void Tempus::Window::SetIcon(const char* path)
{
	int width, height, channels;
	unsigned char* imageData = stbi_load(path, &width, &height, &channels, 4);

	if (!imageData)
	{
		TPS_CORE_WARN("Failed to load icon image: {0}", path);
		return;
	}

	SDL_Surface* iconSurface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, imageData, 4 * width);

	if (!iconSurface)
	{
		TPS_CORE_WARN("Failed to create icon surface: {0}", path);
		stbi_image_free(imageData);
		return;
	}

	if (m_Window) 
	{
		SDL_SetWindowIcon(m_Window, iconSurface);
		SDL_DestroySurface(iconSurface);
		stbi_image_free(imageData);
	}
}

void Tempus::Window::SetFullscreen(bool bEnabled)
{
	if (bEnabled)
	{
		SDL_SetWindowFullscreen(m_Window, true);
	}
	else
	{
		SDL_SetWindowFullscreen(m_Window, false);
	}
}

bool Tempus::Window::IsFullscreen() const
{
	if (m_Window)
	{
		return SDL_GetWindowFlags(m_Window) & SDL_WINDOW_FULLSCREEN;
	}	

	return false;
}

void Tempus::Window::Cleanup()
{
	if (m_Window)
	{
		SDL_DestroyWindow(m_Window);
	}
}



