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

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		void SetIcon(const char* path);
		void SetFullscreen(bool bEnabled);
		bool IsFullscreen() const;

	private:

		SDL_Window* m_Window = nullptr;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		void Cleanup();

	};

}