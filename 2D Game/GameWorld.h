#pragma once
#include "ECS.h"
#include "Level.h"

class GameWorld
{
private:
	World world;
	Level* currentLevel;

public:
	GameWorld() : currentLevel(nullptr)
	{

	}

	void LoadLevel(Level* level)
	{
		currentLevel = level;
	}

	void RenderEntities(SDL_Renderer* renderer)
	{
		world.Query<Transform, Sprite>([&](Entity e, Transform& t, Sprite& s)
			{
				SDL_FRect destRect =
				{
					t.x - s.width / 2,
					t.y - s.height / 2,
					s.width,
					s.height
				};
				SDL_RenderTexture(renderer, s.texture, nullptr, &destRect);
			});
	}

	World& GetWorld() { return world; }
	Level* GetLevel() { return currentLevel; }
};