#pragma once
#include "Level.h"

class LevelEditor
{
private:
	bool isDrawing = false;
	int lastPlacedX = -1;
	int lastPlacedY = -1;
	int currentTileX = 0;
	int currentTileY = 0;
public:
	Level* level; // Points to the level we are editing
	SDL_Renderer* renderer; // We need for drawing grid/overlays, not for the Level itself

	int selectedTile = Level::TILE_AIR; // Default placeable tile (nothing)
	bool showGrid = true; // Grid of all the 16x16 tiles (on by default)
	char saveFilename[256] = "level.1v1"; // Default name for save textbox

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
};