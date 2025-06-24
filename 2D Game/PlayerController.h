#pragma once
#include "ECS.h"
#include <SDL3/SDL.h>

struct Player {
	// Movement settings
	float moveSpeed = 200.0f;
	float jumpForce = 400.0f;
	float maxJumps = 1; 

	// Jump state
	int jumpsRemaining = 1;
	bool jumpHeld = false;
	float jumpHoldTime = 0.0f;
	float maxJumpHoldTime = 0.2f;

	// Coyote time
	float coyoteTime = 0.1f;

	// Input buffer
	float jumpBufferTime = 0.1f;
	float jumpBufferTimer = 0.0f;
};

class PlayerControllerSystem {
public:
    void Update(World& world, float dt) {
        world.Query<Player, Transform, Physics, CollisionState>(
            [dt](Entity e, Player& player, Transform& t, Physics& phys, CollisionState& col) {

                // Get input (we'll make this better later)
                const bool* keys = SDL_GetKeyboardState(nullptr);

                // --- HORIZONTAL MOVEMENT ---
                float moveInput = 0.0f;
                if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
                    moveInput = -1.0f;
                }
                if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
                    moveInput = 1.0f;
                }

                // Apply movement
                phys.velocity.x = moveInput * player.moveSpeed;

                // --- JUMPING ---
                bool jumpPressed = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W];

                // Reset jumps when grounded
                if (col.isGrounded) {
                    player.jumpsRemaining = player.maxJumps;
                }

                // Can we jump? (grounded OR have jumps left OR within coyote time)
                bool canJump = col.isGrounded ||
                    player.jumpsRemaining > 0 ||
                    col.timeSinceGrounded < player.coyoteTime;

                // Jump pressed this frame?
                if (jumpPressed && !player.jumpHeld && canJump) {
                    phys.velocity.y = -player.jumpForce;
                    player.jumpsRemaining--;
                    player.jumpHoldTime = 0.0f;
                }

                // Variable height jump (hold to jump higher)
                if (jumpPressed && player.jumpHeld &&
                    player.jumpHoldTime < player.maxJumpHoldTime &&
                    phys.velocity.y < 0) {
                    // Add a bit more upward force
                    phys.velocity.y -= player.jumpForce * 0.5f * dt;
                    player.jumpHoldTime += dt;
                }

                // Track jump button state
                player.jumpHeld = jumpPressed;
            }
        );
    }

};