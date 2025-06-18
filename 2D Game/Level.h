#pragma once
#include "Array.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Level
{
private:

public:
	// Level constraints
	static constexpr uint8_t TILE_SIZE = 16;
	static constexpr uint16_t MIN_WIDTH = 40;
	static constexpr uint16_t MIN_HEIGHT = 23;
	static constexpr uint16_t MAX_WIDTH = 1000;
	static constexpr uint16_t MAX_HEIGHT = 1000;

	// Actual Level 
	uint16_t MAPWIDTH, MAPHEIGHT;
	Array2D<uint8_t> Tiles;
	Array<SDL_Texture*> tileTextures;

	enum TileType : uint8_t
	{
		// Empty
		TILE_AIR = 0,

		// Spikes (common)
		TILE_SPIKE_UP = 1,
		TILE_SPIKE_DOWN = 2,
		TILE_SPIKE_LEFT = 3,
		TILE_SPIKE_RIGHT = 4,

		// Solid floors
		TILE_GRASS_FLOOR = 5,
		TILE_DIRT_FLOOR = 6,
		TILE_BRICK_FLOOR = 7,
	};

	// Needed for imgui place tile preview 
	SDL_Texture* GetTileTexture(uint8_t tileType)
	{
		return tileTextures[tileType];
	}

	// Used in LoadTextures function to help us load our 256 Tiles
	void LoadTexture(SDL_Renderer* renderer, const char* path, TileType type)
	{
		SDL_Surface* surface = IMG_Load(path);

		if (!surface) 
		{
			SDL_Log("Failed to load %s: %s", path, SDL_GetError());
			return;
		}

		tileTextures[type] = SDL_CreateTextureFromSurface(renderer, surface);

		if (!tileTextures[type]) 
		{
			SDL_Log("Failed to create texture from %s: %s", path, SDL_GetError());
		}
		else 
		{
			// Crispy pixels
			SDL_SetTextureScaleMode(tileTextures[type], SDL_SCALEMODE_NEAREST);
		}

		SDL_DestroySurface(surface);
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
				tileRect.y = (float)(y * TILE_SIZE);
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

		// Fill the bottom row with enum(numbers) for floor testing
		for (int x = 0; x < MAPWIDTH; x++)
		{
			Tiles.Get(x, MAPHEIGHT - 1) = TILE_GRASS_FLOOR;
		}

		for (int x = 0; x < MAPWIDTH; x++)
		{
			Tiles.Get(x, MAPHEIGHT - 3) = TILE_SPIKE_DOWN;
		}

		for (int x = 0; x < MAPWIDTH; x++)
		{
			Tiles.Get(x, MAPHEIGHT - 4) = TILE_BRICK_FLOOR;
		}

	}

	// Free's Gpu memory after the level is done since the textures are stored in the Gpu
	~Level()
	{
		for (int i = 0; i < 256; i++)
		{
			if (tileTextures[i] != nullptr)
			{
				SDL_DestroyTexture(tileTextures[i]);
				tileTextures[i] = nullptr;
			}
		}
	}

	bool SaveToFile(const char* filename)
	{
		SDL_IOStream* file = SDL_IOFromFile(filename, "wb");
		if (!file)
		{
			SDL_Log("Failed to open %s for saving", filename);
			return false;
		}

		SDL_WriteU16BE(file, MAPWIDTH);
		SDL_WriteU16BE(file, MAPHEIGHT);

		for (int y = 0; y < MAPHEIGHT; y++)
		{
			for (int x = 0; x < MAPWIDTH; x++)
			{
				SDL_WriteU8(file, Tiles.Get(x, y));
			}
		}

		SDL_CloseIO(file);
		SDL_Log("Level saved: %s (%dx%d)", filename, MAPWIDTH, MAPHEIGHT);
		return true;
	}

	bool LoadFromFile(const char* filename)
	{
		SDL_IOStream* file = SDL_IOFromFile(filename, "rb");
		if (!file)
		{
			SDL_Log("Failed to open %s for loading", filename);
			return false;
		}

		uint16_t width, height;
		size_t bytesRead;

		bytesRead = SDL_ReadIO(file, &width, sizeof(uint16_t));
		if (bytesRead != sizeof(uint16_t))
		{
			SDL_Log("Failed to read width from %s", filename);
			SDL_CloseIO(file);
			return false;
		}
		width = SDL_Swap16BE(width);

		bytesRead = SDL_ReadIO(file, &height, sizeof(uint16_t));
		if (bytesRead != sizeof(uint16_t))
		{
			SDL_Log("Failed to read height from %s", filename);
			SDL_CloseIO(file);
			return false;
		}
		height = SDL_Swap16BE(height);

		if (width< MIN_WIDTH|| height < MIN_HEIGHT || width > MAX_WIDTH || height > MAX_HEIGHT)
		{
			SDL_Log("Invalid level size: %dx%d (must be between %dx%d and %dx%d)", width, height, MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
			SDL_CloseIO(file);
			return false;
		}

		if (width != MAPWIDTH || height != MAPHEIGHT)
		{
			MAPWIDTH = width;
			MAPHEIGHT = height;
			Tiles = Array2D<uint8_t>(width, height);
		}

		for (int y = 0; y < MAPHEIGHT; y++)
		{
			for (int x = 0; x < MAPWIDTH; x++)
			{
				uint8_t tile;
				bytesRead = SDL_ReadIO(file, &tile, 1);
				if (bytesRead != 1)
				{
					SDL_Log("Failed to read tile at %d,%d", x, y);
					SDL_CloseIO(file);
					return false;
				}
				Tiles.Get(x, y) = tile;
			}
		}

		SDL_CloseIO(file);
		SDL_Log("Level loaded: %s (%dx%d)", filename, MAPWIDTH, MAPHEIGHT);
		return true;
	}

};