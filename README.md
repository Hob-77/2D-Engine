# 2D Platformer Engine
A custom 2D platformer engine built from scratch in C++ featuring a full level editor, ECS architecture, and physics system.

## Features

### Entity Component System (ECS)
- Custom sparse set implementation for efficient component storage
- Support for up to 10,000 entities
- Query system for entities with multiple components
- Components: Transform, Sprite, Animation, Physics, Collider, CollisionState, Player

### Physics System
- AABB collision detection with spatial optimization
- Tile-based world collision
- Layer-based collision filtering (Player, Enemy, Platform, etc.)
- Platformer mechanics:
  - Gravity and velocity-based movement
  - Ground/wall/ceiling detection
  - Coyote time for forgiving jumps
  - Variable jump height
  - Platform inheritance (moving platforms)

### Level Editor (ImGui)
- Real-time tile placement with mouse dragging
- Visual tile palette
- Player spawn point placement
- Play mode testing
- Save/load level files (.lv1 format)
- Camera controls:
  - WASD movement + edge scrolling
  - Zoom with mouse wheel
  - Auto-follow player in play mode
- Grid overlay toggle
- Level boundary visualization

### Rendering
- SDL3 for cross-platform support
- Tile-based rendering with 16x16 tiles
- Efficient culling (only visible tiles rendered)
- Debug visualization options
- Pixel-perfect rendering

## Technical Implementation
- Written in C++ using SDL3 and ImGui
- Fixed timestep physics (60 FPS)
- Spatial optimization for collision checks
- Custom vector math (Vec2)
- All pixel art created in Aseprite

## Dependencies
- SDL3
- SDL3_image
- ImGui
- C++17

## Art
- I made in Asperite!
