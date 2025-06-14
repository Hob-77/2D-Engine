#pragma once
#include "Array.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Level
{
private:

	// Temporary need to change to 16x16 Tiles first for testing
	static constexpr float cameraY = 8.0f;

public:

	static constexpr uint8_t TILE_SIZE = 16;
	uint16_t MAPWIDTH, MAPHEIGHT;
	Array2D<uint8_t> Tiles;
	Array<SDL_Texture*> tileTextures;

	enum TileType : uint8_t
	{
		TILE_AIR = 0,
		TILE_SPIKE_UP = 1,
		TILE_SPIKE_DOWN = 2,
		TILE_SPIKE_LEFT = 3,
		TILE_SPIKE_RIGHT = 4,
		TILE_GRASS_FLOOR = 5,
		TILE_DIRT_FLOOR = 6,
		TILE_BRICK_FLOOR = 7,
	};

	// Used in LoadTextures function to help us load our 256 Tiles
	void LoadTexture(SDL_Renderer* renderer, const char* path, TileType type)
	{
		SDL_Surface* surface = IMG_Load(path);
		if (surface)
		{
			tileTextures[type] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_DestroySurface(surface);
		}
	}

	// Load and attach each texture to the enum above
	void LoadTextures(SDL_Renderer* renderer)
	{
		LoadTexture(renderer, "Assets/spike_up.png", TILE_SPIKE_UP);
		LoadTexture(renderer, "Assets/spike_down.png", TILE_SPIKE_DOWN);
		LoadTexture(renderer, "Assets/spike_left.png", TILE_SPIKE_LEFT);
		LoadTexture(renderer, "Assets/spike_right.png", TILE_SPIKE_RIGHT);
		LoadTexture(renderer, "Assets/grass_floor.png", TILE_GRASS_FLOOR);
		LoadTexture(renderer, "Assets/dirt_floor.png", TILE_DIRT_FLOOR);
		LoadTexture(renderer, "Assets/brick_floor.png", TILE_BRICK_FLOOR);
	}

	// Renders Tiles based on what enum fills the array
	void Render(SDL_Renderer* renderer)
	{
		for (int y = 0; y < MAPHEIGHT; y++)
		{
			for (int x = 0; x < MAPWIDTH; x++)
			{

				uint8_t tileType = Tiles.Get(x, y);

				// skip air tiles
				if (tileType == TILE_AIR || tileTextures[tileType] == nullptr)
				{
					continue;
				}

				SDL_FRect tileRect;
				tileRect.x = (float)(x * TILE_SIZE);
				tileRect.y = (float)(y * TILE_SIZE); // - cameraY for offset
				tileRect.w = (float)TILE_SIZE;
				tileRect.h = (float)TILE_SIZE;

				SDL_RenderTexture(renderer, tileTextures[tileType], nullptr, &tileRect);

			}
		}
	}

	// Creates level
	Level(uint16_t width, uint16_t height) : MAPWIDTH(width),MAPHEIGHT(height),Tiles(width, height), tileTextures(256)
	{

		// Sets the pointers to nullptr
		for (int i = 0; i < 256; i++)
		{
			tileTextures[i] = nullptr;
		}

		// Tiles are now empty
		Tiles.Clear();

		// Fill the bottom row with 1's for floor
		for (int x = 0; x < MAPWIDTH; x++)
		{
			Tiles.Get(x, MAPHEIGHT - 1) = TILE_GRASS_FLOOR;
		}

	}

};