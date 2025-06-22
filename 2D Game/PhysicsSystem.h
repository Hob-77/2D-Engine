#pragma once
#include "ECS.h"
#include "AABB.h"
#include "Level.h"

class PhysicsSystem
{
private:
	float gravity;

public:
	PhysicsSystem(float gravity = 600.0f) : gravity(gravity) {}

	void Update(World& world, Level& level, float dt)
	{
		ApplyForces(world, dt);

		// Step 2: Move entities and resolve collisions
		MoveAndCollide(world, level, dt);
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

    void MoveAndCollide(World& world, Level& level, float dt) {
        // First, clear all collision states
        world.Query<CollisionState>([](Entity e, CollisionState& state) {
            state.Clear();
            });

        // Then do movement and collision detection
        world.Query<Transform, Physics, Collider>([&world, &level, dt](Entity e, Transform& t, Physics& p, Collider& c) {
            if (c.isStatic) return;

            Vec2 movement = p.velocity * dt;

            if (p.isKinematic) {
                t.position += movement;
                return;
            }

            // Get or create collision state
            CollisionState* state = world.GetCollisionState(e);
            if (!state) {
                world.AddCollisionState(e, CollisionState{});
                state = world.GetCollisionState(e);
            }

            Vec2 newPos = t.position;

            // --- HORIZONTAL MOVEMENT ---
            if (movement.x != 0) {
                newPos.x += movement.x;

                AABB horzBox = {
                    newPos + c.offset - c.size * 0.5f,
                    newPos + c.offset + c.size * 0.5f
                };

                // Check tile collision
                if (level.CheckSolidCollision(horzBox)) {
                    if (movement.x > 0) {
                        state->isTouchingWallRight = true;
                        int tileX = (int)((horzBox.max.x) / Level::TILE_SIZE);
                        newPos.x = tileX * Level::TILE_SIZE - c.size.x * 0.5f - c.offset.x - 0.01f;
                    }
                    else {
                        state->isTouchingWallLeft = true;
                        int tileX = (int)((horzBox.min.x) / Level::TILE_SIZE);
                        newPos.x = (tileX + 1) * Level::TILE_SIZE + c.size.x * 0.5f - c.offset.x + 0.01f;
                    }
                    p.velocity.x = 0;
                }

                // Check entity collision
                world.Query<Transform, Collider>([&](Entity other, Transform& otherT, Collider& otherC) {
                    if (e == other || otherC.isTrigger || c.isTrigger) return;

                    // Check layer collision
                    if (!(c.layer & otherC.collidesWith) ||
                        !(otherC.layer & c.collidesWith)) return;

                    AABB otherBox = {
                        otherT.position + otherC.offset - otherC.size * 0.5f,
                        otherT.position + otherC.offset + otherC.size * 0.5f
                    };

                    if (horzBox.intersects(otherBox)) {
                        if (movement.x > 0) {
                            state->isTouchingWallRight = true;
                            newPos.x = otherBox.min.x - c.size.x * 0.5f - c.offset.x - 0.01f;
                        }
                        else {
                            state->isTouchingWallLeft = true;
                            newPos.x = otherBox.max.x + c.size.x * 0.5f - c.offset.x + 0.01f;
                        }

                        // Handle physics response
                        if (!otherC.isStatic) {
                            Physics* otherP = world.GetPhysics(other);
                            if (otherP && !otherP->isKinematic) {
                                // Transfer some momentum
                                otherP->velocity.x += p.velocity.x * 0.5f;
                            }
                        }

                        p.velocity.x = 0;
                    }
                    });
            }

            // --- VERTICAL MOVEMENT ---
            if (movement.y != 0) {
                newPos.y += movement.y;

                AABB vertBox = {
                    newPos + c.offset - c.size * 0.5f,
                    newPos + c.offset + c.size * 0.5f
                };

                // Check tile collision
                if (level.CheckSolidCollision(vertBox)) {
                    if (movement.y > 0) {
                        state->isGrounded = true;
                        state->timeSinceGrounded = 0.0f;
                        int tileY = (int)((vertBox.max.y) / Level::TILE_SIZE);
                        newPos.y = tileY * Level::TILE_SIZE - c.size.y * 0.5f - c.offset.y - 0.01f;
                    }
                    else {
                        state->isTouchingCeiling = true;
                        int tileY = (int)((vertBox.min.y) / Level::TILE_SIZE);
                        newPos.y = (tileY + 1) * Level::TILE_SIZE + c.size.y * 0.5f - c.offset.y + 0.01f;
                    }
                    p.velocity.y = 0;
                }

                // Check entity collision
                world.Query<Transform, Collider>([&](Entity other, Transform& otherT, Collider& otherC) {
                    if (e == other || otherC.isTrigger || c.isTrigger) return;

                    // Check layer collision
                    if (!(c.layer & otherC.collidesWith) ||
                        !(otherC.layer & c.collidesWith)) return;

                    AABB otherBox = {
                        otherT.position + otherC.offset - otherC.size * 0.5f,
                        otherT.position + otherC.offset + otherC.size * 0.5f
                    };

                    if (vertBox.intersects(otherBox)) {
                        if (movement.y > 0) {
                            // Landing on entity
                            state->isGrounded = true;
                            state->groundEntity = other;
                            state->timeSinceGrounded = 0.0f;
                            newPos.y = otherBox.min.y - c.size.y * 0.5f - c.offset.y - 0.01f;

                            // Inherit horizontal velocity from platform
                            Physics* otherP = world.GetPhysics(other);
                            if (otherP && otherP->isKinematic) {
                                p.velocity.x += otherP->velocity.x;
                            }
                        }
                        else {
                            // Hit entity from below
                            state->isTouchingCeiling = true;
                            newPos.y = otherBox.max.y + c.size.y * 0.5f - c.offset.y + 0.01f;
                        }

                        p.velocity.y = 0;
                    }
                    });
            }

            // Update final position
            t.position = newPos;
            });

        // Update timers
        world.Query<CollisionState>([dt](Entity e, CollisionState& state) {
            if (!state.isGrounded) {
                state.timeSinceGrounded += dt;
            }
            if (!state.isTouchingWallLeft && !state.isTouchingWallRight) {
                state.timeSinceWallTouch += dt;
            }
            else {
                state.timeSinceWallTouch = 0.0f;
            }
            });
    }

};