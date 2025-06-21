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

template<typename T>
class SparseSet
{
private:
	static const uint32_t INVALID_INDEX = MAX_ENTITIES + 1;

	Array<uint32_t> sparse; // entity -> dense index
	Array<Entity> dense;
	Array<T> data;
	int count = 0;
	int capacity = 0;

	void Grow()
	{
		int newCapacity = capacity == 0 ? 64 : capacity * 2;

		// Create new arrays
		Array<Entity> newDense(newCapacity);
		Array<T> newData(newCapacity);

		// Copy existing data
		for (int i = 0; i < count; i++)
		{
			newDense[i] = dense[i];
			newData[i] = data[i];
		}

		// Swap arrays
		dense = newDense;
		data = newData;
		capacity = newCapacity;
	}

public:
	SparseSet() : sparse(MAX_ENTITIES), dense(0), data(0), count(0), capacity(0)
	{
		// Initialize sparse array to invalid indices
		for (int i = 0; i < MAX_ENTITIES; i++)
		{
			sparse[i] = INVALID_INDEX;
		}
	}

	void Add(Entity entity, const T& component)
	{
		if (entity >= MAX_ENTITIES)
		{
			return;
		}

		if (Has(entity))
		{
			// Update existing component
			data[sparse[entity]] = component;
			return;
		}

		// Grow if needed
		if (count >= capacity)
		{
			Grow();
		}

		// Add new component
		sparse[entity] = count;
		dense[count] = entity;
		data[count] = component;
		count++;
	}

	void Remove(Entity entity)
	{
		if (!Has(entity))
		{
			return;
		}

		uint32_t index = sparse[entity];
		uint32_t lastIndex = count - 1;

		// Swap with last element if not already last
		if (index != lastIndex)
		{
			Entity lastEntity = dense[lastIndex];

			dense[index] = lastEntity;
			data[index] = data[lastIndex];
			sparse[lastEntity] = index;
		}

		// Mark as removed
		sparse[entity] = INVALID_INDEX;
		count--;
	}

	T* Get(Entity entity)
	{
		if (!Has(entity))
		{
			return nullptr;
		}

		return &data[sparse[entity]];
	}

	bool Has(Entity entity) const
	{
		if (entity >= MAX_ENTITIES)
		{
			return false;
		}
		uint32_t index = sparse[entity];
		return index < count && dense[index] == entity;
	}

	int Count() const
	{
		return count;
	}

	T& GetData(int index)
	{
		return data[index];
	}

	Entity GetEntity(int index) const
	{
		return dense[index];
	}

	// Clear all components
	void Clear()
	{
		for (int i = 0; i < count; i++)
		{
			sparse[dense[i]] = INVALID_INDEX;
		}
		count = 0;
	}

};

class World {
private:
	Entity nextEntity = 1;

	// Sparse sets for each component type
	SparseSet<Transform> transforms;
	SparseSet<Sprite> sprites;
	SparseSet<Animation> animations;
	SparseSet<Velocity> velocities;

public:
	World()
	{

	}
	// Get sparse sets by type
	template<typename T>
	SparseSet<T>* GetSparseSet();

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
		transforms.Add(entity, transform);
	}

	void AddSprite(Entity entity, const Sprite& sprite)
	{
		sprites.Add(entity, sprite);
	}

	void AddAnimation(Entity entity, const Animation& animation)
	{
		animations.Add(entity, animation);
	}

	void AddVelocity(Entity entity, const Velocity& velocity)
	{
		velocities.Add(entity, velocity);
	}

	// Get components
	Transform* GetTransform(Entity entity)
	{
		return transforms.Get(entity);
	}

	Sprite* GetSprite(Entity entity)
	{
		return sprites.Get(entity);
	}

	Animation* GetAnimation(Entity entity)
	{
		return animations.Get(entity);
	}

	Velocity* GetVelocity(Entity entity)
	{
		return velocities.Get(entity);
	}

	// Remove components
	void RemoveTransform(Entity entity)
	{
		transforms.Remove(entity);
	}

	void RemoveSprite(Entity entity)
	{
		sprites.Remove(entity);
	}

	void RemoveAnimation(Entity entity)
	{
		animations.Remove(entity);
	}

	void RemoveVelocity(Entity entity)
	{
		velocities.Remove(entity);
	}

	bool HasTransform(Entity entity)
	{
		return transforms.Has(entity);
	}

	bool HasSprite(Entity entity)
	{
		return sprites.Has(entity);
	}

	bool HasAnimation(Entity entity)
	{
		return animations.Has(entity);
	}

	bool HasVelocity(Entity entity)
	{
		return velocities.Has(entity);
	}

	// Remove components from entity
	void DestroyEntity(Entity entity)
	{
		transforms.Remove(entity);
		sprites.Remove(entity);
		animations.Remove(entity);
		velocities.Remove(entity);
	}

	// Query for entities with 2 components
	template<typename T1, typename T2, typename Func>
	void Query(Func func)
	{
		// Find which sparse set is smaller
		SparseSet<T1>* set1 = GetSparseSet<T1>();
		SparseSet<T2>* set2 = GetSparseSet<T2>();

		// Iterate through the smaller set
		if (set1->Count() <= set2->Count())
		{
			for (int i = 0; i < set1->Count(); i++)
			{
				Entity entity = set1->GetEntity(i);

				// Check if entity has the other component
				T2* comp2 = set2->Get(entity);
				if (comp2)
				{
					T1& comp1 = set1->GetData(i);
					func(entity, comp1, *comp2);
				}
			}
		}
		else
		{
			for (int i = 0; i < set2->Count(); i++)
			{
				Entity entity = set2->GetEntity(i);
				// Check if entity has the other component
				T1* comp1 = set1->Get(entity);
				if (comp1)
				{
					T2& comp2 = set2->GetData(i);
					func(entity, *comp1, comp2);
				}
			}
		}
	}

};

template<> inline SparseSet<Transform>* World::GetSparseSet<Transform>() { return &transforms; }
template<> inline SparseSet<Velocity>* World::GetSparseSet<Velocity>() { return &velocities; }
template<> inline SparseSet<Sprite>* World::GetSparseSet<Sprite>() { return &sprites; }
template<> inline SparseSet<Animation>* World::GetSparseSet<Animation>() { return &animations; }