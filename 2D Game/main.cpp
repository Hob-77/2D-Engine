#include <iostream>

#include "Level.h"
#include "LevelEditor.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"

int windowWidth = 1920;
int windowHeight = 1080;

int main(int argc, char* argv[])
{

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
		return -1;
	}

	SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "0");

	SDL_Window* window = SDL_CreateWindow(
		"2D Platformer",
		windowWidth,
		windowHeight,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
	);

	if (!window)
	{
		std::cerr << "Window could not be created! SDL_ERROR: " << SDL_GetError() << "\n";
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer)
	{
		std::cerr << "Renderer could not be created! SDL_ERROR: " << SDL_GetError() << "\n";
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}

	// Need to comment out for imgui, DO NOT FORGET TO UNDO THIS AFTER ALL THE LEVEL EDITOR STUFF!!!!!
	/*
	SDL_SetRenderLogicalPresentation(renderer, 640, 360, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
	*/

	// ImGui testing
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	Level::InitializeTileProperties();

	// Make level for testing
	LevelEditor editor(renderer, windowWidth, windowHeight);

	// Fixed timestep
	const float FIXED_TIMESTEP = 1.0f / 60.0f;
	const float MAX_FRAME_TIME = 0.25f;

	float accumulator = 0.0f;
	float currentTime = SDL_GetTicks() / 1000.0f;

	bool quit = false;
	SDL_Event event;

	while (!quit)
	{
		// Timing
		float newTime = SDL_GetTicks() / 1000.0f;
		float frameTime = newTime - currentTime;
		currentTime = newTime;

		// Prevent spiral of death
		frameTime = std::min(frameTime, MAX_FRAME_TIME);
		accumulator += frameTime;


		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			editor.HandleInput(event);

			if (event.type == SDL_EVENT_QUIT)
			{
				quit = true;
			}

			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				// Escape to quit
				if (event.key.key == SDLK_ESCAPE)
				{
					quit = true;
				}
			}
		}

		while (accumulator >= FIXED_TIMESTEP)
		{

			editor.UpdateWorld(FIXED_TIMESTEP);

			accumulator -= FIXED_TIMESTEP;
		}

		// Update for movement of screen in editor
		editor.Update();

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		editor.DrawUI();

		SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
		SDL_RenderClear(renderer);

		editor.RenderWithCamera();

		// Grid and Tile preview
		editor.Draw();

		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

		SDL_RenderPresent(renderer);
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}