#include "Renderer.h"

#include "sdl/SDL.h"
#include "Window.h"

Tempus::Renderer::Renderer()
{
}

Tempus::Renderer::~Renderer()
{
}

bool Tempus::Renderer::Init(Tempus::Window* window)
{

	if (!window) 
	{
		return false;
	}

	m_Renderer = SDL_CreateRenderer(window->GetNativeWindow(), -1, 0);
	SDL_SetRenderDrawColor(m_Renderer, 19, 61, 102, 255);

	return m_Renderer;

}

int Tempus::Renderer::RenderClear()
{
	return SDL_RenderClear(m_Renderer);
}

void Tempus::Renderer::RenderPresent()
{
	SDL_RenderPresent(m_Renderer);
}

int Tempus::Renderer::SetRenderDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return SDL_SetRenderDrawColor(m_Renderer, r, g, b, a);
}
