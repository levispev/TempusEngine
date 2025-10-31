// Copyright Levi Spevakow (C) 2025

#pragma once

#include <memory>

#include "Tempus/Core/Core.h"
#include "sdl/SDL.h"

namespace Tempus {

	class TEMPUS_API Window
	{
	public:

		Window();
		~Window();

		bool Init(const char* title,
			int x, int y, int w,
			int h, Uint32 flags);

		SDL_Window* GetNativeWindow() const;

		void SetIcon(const char* path);

	private:

		SDL_Window* m_Window = nullptr;

		void Cleanup();

	};

}