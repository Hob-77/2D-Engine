#include <iostream>
#include <chrono>

#include "Level.h"
#include "LevelEditor.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"

int windowWidth = 1920;
int windowHeight = 1080;

// Global performance file handle
FILE* g_performanceFile = nullptr;

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

	// Open global performance file
	fopen_s(&g_performanceFile, "performance_data.csv", "w");
	if (!g_performanceFile) {
		std::cerr << "Failed to create performance_data.csv\n";
	}

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

	// Frame timing
	auto frameStart = std::chrono::high_resolution_clock::now();

	while (!quit)
	{
		// Time total frame
		auto frameEnd = std::chrono::high_resolution_clock::now();
		auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
		if (g_performanceFile) {
			fprintf(g_performanceFile, "TOTAL_FRAME,%ld\n", frameDuration.count());
			fflush(g_performanceFile);
		}
		frameStart = frameEnd;

		// Timing
		float newTime = SDL_GetTicks() / 1000.0f;
		float frameTime = newTime - currentTime;
		currentTime = newTime;

		// Prevent spiral of death
		frameTime = std::min(frameTime, MAX_FRAME_TIME);
		accumulator += frameTime;

		// Time input handling
		auto start = std::chrono::high_resolution_clock::now();
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
				if (event.key.key == SDLK_ESCAPE)
				{
					quit = true;
				}
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		if (g_performanceFile) {
			fprintf(g_performanceFile, "INPUT_HANDLING,%ld\n", duration.count());
			fflush(g_performanceFile);
		}

		// Time physics updates
		start = std::chrono::high_resolution_clock::now();
		while (accumulator >= FIXED_TIMESTEP)
		{
			editor.UpdateWorld(FIXED_TIMESTEP);
			accumulator -= FIXED_TIMESTEP;
		}
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		if (g_performanceFile) {
			fprintf(g_performanceFile, "FIXED_UPDATE,%ld\n", duration.count());
			fflush(g_performanceFile);
		}

		// Time editor update (camera movement)
		start = std::chrono::high_resolution_clock::now();
		editor.Update();
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		if (g_performanceFile) {
			fprintf(g_performanceFile, "EDITOR_UPDATE,%ld\n", duration.count());
			fflush(g_performanceFile);
		}

		// Time ImGui preparation
		start = std::chrono::high_resolution_clock::now();
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		editor.DrawUI();
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		if (g_performanceFile) {
			fprintf(g_performanceFile, "IMGUI_UPDATE,%ld\n", duration.count());
			fflush(g_performanceFile);
		}

		// Time rendering
		start = std::chrono::high_resolution_clock::now();
		SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
		SDL_RenderClear(renderer);

		editor.RenderWithCamera();
		editor.Draw();

		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		if (g_performanceFile) {
			fprintf(g_performanceFile, "RENDER,%ld\n", duration.count());
			fflush(g_performanceFile);
		}
	}

	// Close performance file
	if (g_performanceFile) {
		fclose(g_performanceFile);
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}