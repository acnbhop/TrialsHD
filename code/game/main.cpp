//
// main.cpp
//

#include "app.hpp"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	redlynx::game::Application ApplicationInstance;
	redlynx::game::ApplicationConfiguration Configuration;
	Configuration.Title = "Trials HD";
	Configuration.Width = 1280;
	Configuration.Height = 720;
	Configuration.Resizable = true;
	Configuration.Fullscreen = false;

	if (!ApplicationInstance.Init(Configuration))
	{
		SDL_Log("[Main] Failed to initialize application.");
		return EXIT_FAILURE;
	}

	ApplicationInstance.Run();

	// Shutdown is called by the App deconstructor, but we are
	// also going to make it explicit about it here.
	ApplicationInstance.Shutdown();

	return EXIT_SUCCESS;
}
