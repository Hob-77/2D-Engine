#include <iostream>

#include "Player.h"
#include "Level.h"
#include "Array.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

int main(int argc, char* argv[])
{

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"2D Platformer",
		1920,
		1080,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
	);

	if (!window)
	{
		std::cout << "Window could not be created! SDL_ERROR: " << SDL_GetError() << "\n";
		SDL_Quit();
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer)
	{
		std::cout << "Renderer could not be created! SDL_ERROR: " << SDL_GetError() << "\n";
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}

	SDL_SetRenderLogicalPresentation(renderer, 640, 360, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

	// Make level for testing
	Level level1(40, 23);
	level1.LoadTextures(renderer);

	bool quit = false;
	SDL_Event event;

	while (!quit)
	{

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				quit = true;
			}

			if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
			{
				quit = true;
			}
		}

		SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255);
		SDL_RenderClear(renderer);

		level1.Render(renderer);

		SDL_RenderPresent(renderer);

	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;

}