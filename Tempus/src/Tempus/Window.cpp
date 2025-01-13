// Copyright Levi Spevakow (C) 2025

#include "Window.h"

namespace Tempus {

	Window::Window()
	{
	}

	Window::~Window()
	{
		if (m_Window) 
		{
			SDL_DestroyWindow(m_Window);
		}
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

}

