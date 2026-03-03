//
// app.hpp
//

#pragma once

// Engine headers
#include "engine/renderer.hpp"

// Standard headers
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_GAME
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ApplicationConfiguration
{
	std::string Title = "Trials HD";
	uint32 Width = 1280;
	uint32 Height = 720;
	bool Fullscreen = false;
	bool Resizable = true;
	bool VSync = true;
};

class Application
{
public:
	Application();
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	///
	/// @brief Initialize the application.
	/// @param ApplicationConfiguration The application configuration to use for initialization.
	///
	bool Init(const ApplicationConfiguration& Configuration = {});
	void Shutdown();
	void Run();

	bool IsRunning() const { return m_Running; }
	engine::Renderer& GetRenderer() { return m_Renderer; }
	const engine::Renderer& GetRenderer() const { return m_Renderer; }
	SDL_Window* GetWindow() const { return m_Window; }
private:
	void _ProcessEvents();
	void _Update(f32 DeltaTime);
	void _Render();

	SDL_Window* m_Window = nullptr;
	engine::Renderer m_Renderer;
	bool m_Running = false;
	bool m_Initialized = false;
	uint64 m_LastFrameTime = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_GAME
////////////////////////////////////////////////////////////////////////////////////////////////////
