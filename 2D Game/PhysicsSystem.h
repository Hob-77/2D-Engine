#pragma once
#include "ECS.h"
#include "AABB.h"

class PhysicsSystem
{
private:
	float gravity;

public:
	PhysicsSystem(float gravity = 600.0f) : gravity(gravity) {}

	void Update(World& world, float dt)
	{
		ApplyForces(world, dt);

		// Step 2: Move entities and resolve collisions
		MoveAndCollide(world, dt);
	}

    void ApplyForces(World& world, float dt) {
        world.Query<Physics>([this, dt](Entity e, Physics& p) {
            if (p.isKinematic) return;  // Kinematic = no forces

            // Apply gravity
            p.acceleration.y += gravity * p.gravityScale;

            // Update velocity
            p.velocity += p.acceleration * dt;

            // Apply damping
            p.velocity *= (1.0f - p.linearDamping * dt);

            // Clamp fall speed
            if (p.velocity.y > p.maxFallSpeed) {
                p.velocity.y = p.maxFallSpeed;
            }

            // Reset acceleration
            p.acceleration = Vec2(0, 0);
            });
    }

    void MoveAndCollide(World& world, float dt) {
        // We need entities with Transform, Physics, AND Collider
        world.Query<Transform, Physics, Collider>([&world, dt](Entity e, Transform& t, Physics& p, Collider& c) {
            // Skip static colliders - they don't move
            if (c.isStatic) return;

            // Calculate desired movement
            Vec2 movement = p.velocity * dt;

            // For kinematic objects, just move them
            if (p.isKinematic) {
                t.position += movement;
                return;
            }

            // For dynamic objects, we need to check collisions
            // We'll build this next...
            });
    }


private:
	void ApplyForces(World& world, float dt);
	void MoveAndCollide(World& world, float dt);
};