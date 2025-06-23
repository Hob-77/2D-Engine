#pragma once
#include "Array.h"
#include "AABB.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Level
{
public:
	struct TileProperties
	{
		bool isSolid;
		AABB collisionBox;
		bool isDamaging;
		uint8_t damageAmount;

		// Default: empty tile
		TileProperties() :
			isSolid(false),
			collisionBox({ Vec2(0,0), Vec2(16,16) }),
			isDamaging(false),
			damageAmount(0) {
		}

	};

private:
	static TileProperties tileProperties[256];
public:
	// Level constraints
	static constexpr uint8_t TILE_SIZE = 16;
	// One screen size on modern displays
	static constexpr uint16_t MIN_WIDTH = 40;
	static constexpr uint16_t MIN_HEIGHT = 23;
	// Huge level maximum size
	static constexpr uint16_t MAX_WIDTH = 1000;
	static constexpr uint16_t MAX_HEIGHT = 1000;

	// Actual Level 
	uint16_t MAPWIDTH, MAPHEIGHT;
	Array2D<uint8_t> Tiles;
	Array<SDL_Texture*> tileTextures;

	// Player spawn point
	Vec2 playerSpawnPoint;
	bool hasPlayerSpawn = false;

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

	// Getter: Needed for ImGui
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
		Tiles.Clear(TILE_AIR);

		playerSpawnPoint = Vec2(
			(width * TILE_SIZE) / 2.0f,
			(height * TILE_SIZE) / 2.0f
		);
		hasPlayerSpawn = true;
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

		// Player spawn point
		SDL_WriteU8(file, hasPlayerSpawn ? 1 : 0);
		if (hasPlayerSpawn)
		{
			// write as floats
			float x = playerSpawnPoint.x;
			float y = playerSpawnPoint.y;
			SDL_WriteIO(file, &x, sizeof(float));
			SDL_WriteIO(file, &y, sizeof(float));
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

		// Load Player spawn
		uint8_t hasSpawn;
		bytesRead = SDL_ReadIO(file, &hasSpawn, 1);
		if (bytesRead == 1 && hasSpawn)
		{
			float x, y;
			SDL_ReadIO(file, &x, sizeof(float));
			SDL_ReadIO(file, &y, sizeof(float));
			playerSpawnPoint = Vec2(x, y);
			hasPlayerSpawn = true;
		}

		SDL_CloseIO(file);
		SDL_Log("Level loaded: %s (%dx%d)", filename, MAPWIDTH, MAPHEIGHT);
		return true;
	}


	static void InitializeTileProperties()
	{
		// Air - no collision
		tileProperties[TILE_AIR] = TileProperties();

		// Solid floors - full tile collision
		tileProperties[TILE_GRASS_FLOOR].isSolid = true;
		tileProperties[TILE_DIRT_FLOOR].isSolid = true;
		tileProperties[TILE_BRICK_FLOOR].isSolid = true;


	}

	static const TileProperties& GetTileProperties(uint8_t tileType)
	{
		return tileProperties[tileType];
	}

	bool CheckSolidCollision(const AABB& box) const
	{
		// convert to tile coordinates
		int startX = std::max(0, (int)(box.min.x / TILE_SIZE));
		int startY = std::max(0, (int)(box.min.y / TILE_SIZE));
		int endX = std::min((int)MAPWIDTH - 1, (int)(box.max.x / TILE_SIZE));
		int endY = std::min((int)MAPHEIGHT - 1, (int)(box.max.y / TILE_SIZE));

		for (int y = startY; y <= endY; y++)
		{
			for (int x = startX; x <= endX; x++)
			{
				uint8_t tileType = Tiles.Get(x, y);
				const TileProperties& props = GetTileProperties(tileType);

				if (props.isSolid)
				{
					AABB tileBox = AABB::FromPositionSize(
						Vec2(x * TILE_SIZE, y * TILE_SIZE),
						Vec2(TILE_SIZE, TILE_SIZE)
					);

					if (box.intersects(tileBox))
					{
						return true;
					}
				}

			}
		}
		return false;
	}

};

inline Level::TileProperties Level::tileProperties[256];