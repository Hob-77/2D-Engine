#pragma once
#include "Array.h"
#include <SDL3/SDL.h>

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

	Level(uint16_t width, uint16_t height) : MAPWIDTH(width),MAPHEIGHT(height),Tiles(width, height), tileTextures(256)
	{
		Tiles.Clear(); // Tiles are now empty

		// Fill the bottom row with 1's for floor
		for (int x = 0; x < MAPWIDTH; x++)
		{
			Tiles.Get(x, MAPHEIGHT - 1) = TILE_FLOOR;
		}
	}

};