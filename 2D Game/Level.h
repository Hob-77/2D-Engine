#pragma once
#include "Array.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Level
{
public:

	static constexpr uint8_t TILE_SIZE = 32;
	uint16_t MAPWIDTH, MAPHEIGHT;
	Array2D<uint8_t> Tiles;
	Array<SDL_Texture*> tileTextures;

	enum TileType : uint8_t
	{
		TILE_AIR = 0,
		TILE_FLOOR = 1,
		TILE_SPIKE = 2,
	};

	void LoadTextures(SDL_Renderer* renderer)
	{

		// Load and attach each texture to the enum above
		SDL_Surface* surface = IMG_Load("Assets/floor.png");
		if (surface)
		{
			tileTextures[TILE_FLOOR] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_DestroySurface(surface);
		}

	}

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

				SDL_FRect destRect;
				destRect.x = (float)(x * TILE_SIZE);
				destRect.y = (float)(y * TILE_SIZE);
				destRect.w = (float)TILE_SIZE;
				destRect.h = (float)TILE_SIZE;

				SDL_RenderTexture(renderer, tileTextures[tileType], nullptr, &destRect);

			}
		}
	}

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
			Tiles.Get(x, MAPHEIGHT - 1) = TILE_FLOOR;
		}

	}

};