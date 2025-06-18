#pragma once
#include "Level.h"

class LevelEditor
{
private:
	// For drawing and placing tiles
	bool isDrawing = false;
	int lastPlacedX = -1;
	int lastPlacedY = -1;
	int currentTileX = 0;
	int currentTileY = 0;

	// For window dimensions
	int windowWidth;
	int windowHeight;

	// Delta time for camera
	Uint64 lastFrameTime;
	float deltaTime;

	// Movement for screen and zoom
	static constexpr float EDGE_SCROLL_MARGIN = 50.0f;
	static constexpr float EDGE_SCROLL_SPEED = 400.0f;
	static constexpr float MIN_ZOOM = 0.25f;
	static constexpr float MAX_ZOOM = 4.0f;

public:
	// Editor camera
	float cameraX, cameraY;
	float cameraZoom;

	Level* level; // Points to the level we are editing
	SDL_Renderer* renderer; // We need for drawing grid/overlays, not for the Level itself

	int selectedTile = Level::TILE_AIR; // Default placeable tile (nothing)
	bool showGrid = true; // Grid of all the 16x16 tiles (on by default)
	char saveFilename[256] = "level.1v1"; // Default name for save textbox

	// constructor
	LevelEditor(SDL_Renderer* r, Level* lvl, int windowW, int windowH) : renderer(r), level(lvl), windowWidth(windowW), windowHeight(windowH)
	{
		// Initialize camera
		cameraZoom = 1.0f;

		// Center camera on the level
		cameraX = (level->MAPWIDTH * Level::TILE_SIZE) / 2.0f - (windowWidth / 2.0f);
		cameraY = (level->MAPHEIGHT * Level::TILE_SIZE) / 2.0f - (windowHeight / 2.0f);

		lastFrameTime = SDL_GetTicks();
		deltaTime = 0.0f;

	}

	// Editor functions
	void HandleInput(SDL_Event& event);
	void Draw();
	void DrawUI();
	void DrawGrid();
	void PlaceTile(int x, int y);
	void PlaceTilePreview(int x, int y);
	void SaveLevel();
	void LoadLevel();
	void CreateNewLevel(int width, int height);

	// Camera functions
	void RecenterCamera();
	void ScreenToWorld(float screenX, float screenY, float& worldX, float& worldY);
	void RenderWithCamera();
	void UpdateCamera(float deltaTime);
	void Update();

};