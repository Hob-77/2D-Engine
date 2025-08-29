## 2D Game Engine

Custom game engine systems built from scratch in C++ with imGui for integrated level editor. Using SDL3 for windowing, input handeling, and rendering.

https://www.youtube.com/watch?v=iY-0GqAhtyY

### Technical Features
- **Entity Component System (ECS)**
  - Sparse set implementation for cache efficiency
  - Stress tested with 10,000+ concurrent entities at 60 FPS
  - Template-based query system for component iteration

https://www.youtube.com/watch?v=W9LZiqMyiVk

- **Level Editor** 
  - Real-time editing with Dear ImGui
  - Hot-swappable play/edit modes
  - Tile-based level serialization
  - Visual player spawn placement

- **Physics System**
  - AABB collision detection with spatial partitioning
  - Platform physics with coyote time
  - Variable jump height mechanics
  - Per-entity gravity and damping

- **Rendering**
  - Camera system with smooth lag following
  - Zoom controls for editor
  - Optimized tile culling (only renders visible)
  - 16x16 Tileset internal rendering set at 640x360 (16:9) integer scaled to 1920x1080 or any modern desktop size.

### Architecture Highlights
- Cache-friendly component storage with sparse sets
- Fixed timestep game loop preventing spiral of death
- Clean separation of systems (Physics, Rendering, Input)
- Zero heap allocations in hot paths

### Performance
- 10,000 entities with unique physics properties at 60 FPS
- Efficient spatial queries for collision detection
- Minimal draw calls through batching

### In Development
- Animation system
- Enemy AI
- Particle systems
- Texture Manager

### Bonus
- All art made in Asperite by me

### Build Requirements
- SDL3
- Dear ImGui
- C++20
