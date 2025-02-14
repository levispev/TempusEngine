// Copyright Levi Spevakow (C) 2025

#include "Window.h"

#include "Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

namespace Tempus {

	Window::Window()
	{
	}

	Window::~Window()
	{
		Cleanup();
	}

	bool Window::Init(const char* title,
		int x, int y, int w,
		int h, Uint32 flags)
	{

		m_Window = SDL_CreateWindow(title, x, y, w, h, flags);

		return m_Window;
	}

	SDL_Window* Window::GetNativeWindow() const
	{
		return m_Window;
	}

	void Window::SetIcon(const char* path)
	{
		int width, height, channels;
		unsigned char* imageData = stbi_load(path, &width, &height, &channels, 4);

		if (!imageData)
		{
			TPS_CORE_WARN("Failed to load icon image: {0}", path);
			return;
		}

		SDL_Surface* iconSurface = SDL_CreateRGBSurfaceWithFormatFrom(imageData, width, height, 32, 4 * width, SDL_PIXELFORMAT_RGBA32);

		if (!iconSurface)
		{
			TPS_CORE_WARN("Failed to create icon surface: {0}", path);
			stbi_image_free(imageData);
			return;
		}

		if (m_Window) 
		{
			SDL_SetWindowIcon(m_Window, iconSurface);
			SDL_FreeSurface(iconSurface);
			stbi_image_free(imageData);
		}
	}

	void Window::Cleanup()
	{
		if (m_Window)
		{
			SDL_DestroyWindow(m_Window);
		}
	}

}

