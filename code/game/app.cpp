//
// app.cpp
//

// File header
#include "app.hpp"

// Engine headers
#include "engine/renderer.hpp"

// SDL headers
#include <SDL3/SDL_timer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_GAME
////////////////////////////////////////////////////////////////////////////////////////////////////

Application::Application()
{
}

Application::~Application()
{
	Shutdown();
}

bool Application::Init(const ApplicationConfiguration& Configuration)
{
	if (m_Initialized)
	{
		SDL_Log("[Game] [App] Already initialized.");
		return false;
	}

	//
	// Startup SDL
	//
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD))
	{
		SDL_Log("[Game] [App] Failed to initialize SDL: %s", SDL_GetError());
		return false;
	}

	//
	// Build the window flags.
	//
	SDL_WindowFlags WindowFlags = 0;

	if (Configuration.Resizable)
	{
		WindowFlags |= SDL_WINDOW_RESIZABLE;
	}

	if (Configuration.Fullscreen)
	{
		WindowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	//
	// Create the window
	//
	m_Window = SDL_CreateWindow(Configuration.Title.c_str(), static_cast<int>(Configuration.Width), static_cast<int>(Configuration.Height), WindowFlags);
	if (!m_Window)
	{
		SDL_Log("[Game] [App] Failed to create window: %s", SDL_GetError());
		SDL_Quit();
		return false;
	}

	//
	// Initialize renderer with window.
	//
	if (!m_Renderer.Init(m_Window))
	{
		SDL_Log("[Game] [App] Failed to initialize renderer.");
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		return false;
	}

	SDL_Log("[Game] [App] Initialized successfully.");

	m_Initialized = true;
	m_Running = true;
	m_LastFrameTime = SDL_GetPerformanceCounter();

	return true;
}

void Application::Shutdown()
{
	if (!m_Initialized)
	{
		SDL_Log("[Game] [App] Not initialized, skipping shutdown.");
		return;
	}

	m_Running = false;
	m_Renderer.Shutdown();
	if (m_Window)
	{
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
	}
	SDL_Quit();
	m_Initialized = false;
	SDL_Log("[Game] [App] Shutdown complete.");
}

void Application::Run()
{
	while (m_Running)
	{
		uint64 Now = SDL_GetPerformanceCounter();
		f64 Frequency = static_cast<f64>(SDL_GetPerformanceFrequency());
		f32 DeltaTime = static_cast<f32>(static_cast<f64>(Now - m_LastFrameTime) / Frequency);
		m_LastFrameTime = Now;

		// Clamp delta time to avoid issues after breakpoinst / hitches.
		if (DeltaTime > 0.25f)
		{
			DeltaTime = 0.25f;
		}

		_ProcessEvents();

		if (!m_Running)
		{
			break;
		}

		_Update(DeltaTime);
		_Render();
	}
}

void Application::_ProcessEvents()
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		switch (Event.type)
		{
			case SDL_EVENT_QUIT:
				m_Running = false;
				break;
			case SDL_EVENT_KEY_DOWN:
				if (Event.key.key == SDLK_ESCAPE)
				{
					m_Running = false;
				}
				break;
			default:
				break;
		}
	}
}

void Application::_Update(f32 DeltaTime)
{
	(void)DeltaTime;
}

void Application::_Render()
{
	if (!m_Renderer.BeginFrame())
	{
		SDL_Log("[Game] [App] Failed to begin frame.");
		return;
	}

	//
	// Main render pass to the swapchain.
	//
	engine::RenderPassConfiguration PassConfiguration;
	PassConfiguration.ClearColor[0] = 0.02f;
	PassConfiguration.ClearColor[1] = 0.02f;
	PassConfiguration.ClearColor[2] = 0.04f;
	PassConfiguration.ClearColor[3] = 1.0f;
	PassConfiguration.ClearColorBuffer = true;
	PassConfiguration.ClearDepthBuffer = true;

	if (m_Renderer.BeginRenderPass(PassConfiguration))
	{
		// TODO: Rendering goes here.

		m_Renderer.EndRenderPass();
	}

	m_Renderer.EndFrame();
}
REDLYNX_NAMESPACE_END_GAME
