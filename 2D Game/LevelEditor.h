#pragma once
#include "Level.h"

class LevelEditor
{
private:
	// Default level
	Level* level;

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
	static constexpr float DEFAULT_ZOOM = 1.0f;
	static constexpr float MAX_ZOOM = 4.0f;

	// Editor camera
	float cameraX, cameraY;
	float cameraZoom;

public:
	SDL_Renderer* renderer; // We need for drawing grid/overlays, not for the Level itself

	int selectedTile = Level::TILE_AIR; // Default placeable tile (nothing)
	bool showGrid = true; // Grid of all the 16x16 tiles (on by default)
	char saveFilename[256] = "level.1v1"; // Default name for save textbox

	// Constructor
	LevelEditor(SDL_Renderer* r, int windowW, int windowH) : renderer(r), windowWidth(windowW), windowHeight(windowH)
	{

		// Default Level
		level = new Level(Level::MIN_WIDTH, Level::MIN_HEIGHT);
		level->LoadTextures(renderer);

		// Initialize camera
		ResetZoom();
		RecenterCamera();

		lastFrameTime = SDL_GetTicks();
		deltaTime = 0.0f;

	}

	// Destructor
	~LevelEditor()
	{
		delete level;
	}

	// Editor functions
	void HandleInput(SDL_Event& event);
	void Draw();
	void DrawUI();
	void DrawGrid();
	void DrawLevelBoundary();
	void PlaceTile(int x, int y);
	void PlaceTilePreview(int x, int y);
	void SaveLevel();
	void LoadLevel();
	void CreateNewLevel(int width, int height);

	// Camera functions
	void RecenterCamera();
	void ResetZoom();
	void ScreenToWorld(float screenX, float screenY, float& worldX, float& worldY);
	void RenderWithCamera();
	void UpdateCamera(float deltaTime);
	void Update();

};