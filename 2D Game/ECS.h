#pragma once
#include <cstdint>
#include "Array.h"
#include <SDL3/SDL.h>

// Entity
using Entity = std::uint32_t;
const Entity NULL_ENTITY = 0;
const int MAX_ENTITIES = 10000;

enum ComponentType {
	COMPONENT_TRANSFORM,
	COMPONENT_SPRITE,
	COMPONENT_ANIMATION,
	COMPONENT_VELOCITY,
	MAX_COMPONENTS
};

// Components
struct Transform {
	float x = 0;
	float y = 0;
	float rotation = 0;
	float scale = 1;
};

struct Sprite {
	SDL_Texture* texture = nullptr;
	SDL_Color color = { 255,255,255,255 };
	uint8_t width = 0;
	uint8_t height = 0;
};

struct Animation {
	SDL_Texture* spriteSheet = nullptr;
	uint8_t frameWidth = 0;
	uint8_t frameHeight = 0;
	uint8_t currentFrame = 0;
	uint8_t totalFrames = 0;
	float frameTime = 0.1f;
	float timer = 0.0f;
	bool loop = true;
};

struct Velocity {
	float dx = 0;
	float dy = 0;
};

class World {
private:
	Entity nextEntity = 1;

	Array<Transform> transforms;
	Array<Sprite> sprites;
	Array<Animation> animations;
	Array<Velocity> velocities;

	Array<bool> hasTransform;
	Array<bool> hasSprite;
	Array<bool> hasAnimation;
	Array<bool> hasVelocity;

public:
	World() :
		transforms(MAX_ENTITIES),
		sprites(MAX_ENTITIES),
		animations(MAX_ENTITIES),
		velocities(MAX_ENTITIES),
		hasTransform(MAX_ENTITIES),
		hasSprite(MAX_ENTITIES),
		hasAnimation(MAX_ENTITIES),
		hasVelocity(MAX_ENTITIES)
	{

		for (int i = 0; i < MAX_ENTITIES; i++)
		{
			hasTransform[i] = false;
			hasSprite[i] = false;
			hasAnimation[i] = false;
			hasVelocity[i] = false;
		}
	}

	// Make Entity
	Entity CreateEntity()
	{
		if (nextEntity >= MAX_ENTITIES)
		{
			return NULL_ENTITY;
		}
		return nextEntity++;
	}

	// Add components

	void AddTransform(Entity entity, const Transform& transform)
	{
		transforms[entity] = transform;
		hasTransform[entity] = true;
	}

	void AddSprite(Entity entity, const Sprite& sprite)
	{
		sprites[entity] = sprite;
		hasSprite[entity] = true;
	}

	void AddAnimation(Entity entity, const Animation& animation)
	{
		animations[entity] = animation;
		hasAnimation[entity] = true;
	}

	void AddVelocity(Entity entity, const Velocity& velocity)
	{
		velocities[entity] = velocity;
		hasVelocity[entity] = true;
	}

	// Get components
	Transform* GetTransform(Entity entity)
	{
		if (entity < MAX_ENTITIES && hasTransform[entity])
		{
			return &transforms[entity];
		}
		return nullptr;
	}

	Sprite* GetSprite(Entity entity)
	{
		if (entity < MAX_ENTITIES && hasSprite[entity])
		{
			return &sprites[entity];
		}
		return nullptr;
	}

	Animation* GetAnimation(Entity entity)
	{
		if (entity < MAX_ENTITIES && hasAnimation[entity])
		{
			return &animations[entity];
		}
		return nullptr;
	}

	Velocity* GetVelocity(Entity entity)
	{
		if (entity < MAX_ENTITIES && hasVelocity[entity])
		{
			return &velocities[entity];
		}
		return nullptr;
	}

	// Remove components
	void RemoveTransform(Entity entity)
	{
		hasTransform[entity] = false;
	}

	void RemoveSprite(Entity entity)
	{
		hasSprite[entity] = false;
	}

	void RemoveAnimation(Entity entity)
	{
		hasAnimation[entity] = false;
	}

	void RemoveVelocity(Entity entity)
	{
		hasVelocity[entity] = false;
	}

	bool HasTransform(Entity entity)
	{
		return entity < MAX_ENTITIES && hasTransform[entity];
	}

	bool HasSprite(Entity entity)
	{
		return entity < MAX_ENTITIES && hasSprite[entity];
	}

	bool HasAnimation(Entity entity)
	{
		return entity < MAX_ENTITIES && hasAnimation[entity];
	}

	bool HasVelocity(Entity entity)
	{
		return entity < MAX_ENTITIES && hasVelocity[entity];
	}

	// Remove components from entity
	void DestroyEntity(Entity entity)
	{
		if (entity < MAX_ENTITIES)
		{
			hasTransform[entity] = false;
			hasSprite[entity] = false;
			hasAnimation[entity] = false;
			hasVelocity[entity] = false;
		}
	}

};