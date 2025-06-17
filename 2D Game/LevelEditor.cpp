#include "LevelEditor.h"
#include "imgui/imgui.h"

// Level Editor imgui input
void LevelEditor::HandleInput(SDL_Event& event)
{
	ImGuiIO& io = ImGui::GetIO();

	// Track mouse position
	if (event.type == SDL_EVENT_MOUSE_MOTION)
	{
		if (!io.WantCaptureMouse)
		{
			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			currentTileX = (int)(mouseX / Level::TILE_SIZE);
			currentTileY = (int)(mouseY / Level::TILE_SIZE);

			// Place tile while dragging
			if (isDrawing)
			{
				// Only place if we moved to a new tile
				if (currentTileX != lastPlacedX || currentTileY != lastPlacedY)
				{
					PlaceTile(currentTileX, currentTileY);
					lastPlacedX = currentTileX;
					lastPlacedY = currentTileY;
				}
			}
		}
	}



	// Left click place tile, hold left click drag to place quickly
	if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
	{
		// Check to see if mouse click
		if (!io.WantCaptureMouse)
		{
			isDrawing = true;

			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			int tileX = (int)(mouseX / Level::TILE_SIZE);
			int tileY = (int)(mouseY / Level::TILE_SIZE);

			PlaceTile(tileX, tileY);
			lastPlacedX = tileX;
			lastPlacedY = tileY;
		}
	}

	// Check to stop left click hold drawing
	if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT)
	{
		isDrawing = false;
		lastPlacedX = -1;
		lastPlacedY = -1;
	}

}

void LevelEditor::DrawUI()
{
	// Main editor window
	ImGui::Begin("Level Editor");

	// Tile selector
	ImGui::Text("Select Tile:");
	ImGui::Separator();

	if (ImGui::RadioButton("Air (Erase)", selectedTile == Level::TILE_AIR))
	{
		selectedTile = Level::TILE_AIR;
	}
	if (ImGui::RadioButton("Spike Up", selectedTile == Level::TILE_SPIKE_UP))
	{
		selectedTile = Level::TILE_SPIKE_UP;
	}
	if (ImGui::RadioButton("Spike Down", selectedTile == Level::TILE_SPIKE_DOWN))
	{
		selectedTile = Level::TILE_SPIKE_DOWN;
	}
	if (ImGui::RadioButton("Spike Left", selectedTile == Level::TILE_SPIKE_LEFT))
	{
		selectedTile = Level::TILE_SPIKE_LEFT;
	}
	if (ImGui::RadioButton("Spike Right", selectedTile == Level::TILE_SPIKE_RIGHT))
	{
		selectedTile = Level::TILE_SPIKE_RIGHT;
	}
	if (ImGui::RadioButton("Grass Floor", selectedTile == Level::TILE_GRASS_FLOOR))
	{
		selectedTile = Level::TILE_GRASS_FLOOR;
	}
	if (ImGui::RadioButton("Dirt Floor", selectedTile == Level::TILE_DIRT_FLOOR))
	{
		selectedTile = Level::TILE_DIRT_FLOOR;
	}
	if (ImGui::RadioButton("Brick Floor", selectedTile == Level::TILE_BRICK_FLOOR))
	{
		selectedTile = Level::TILE_BRICK_FLOOR;
	}

	ImGui::Separator();

	// Grid toggle
	ImGui::Checkbox("Show Grid", &showGrid);
	ImGui::Separator();

	ImGui::Text("File Operations:");
	ImGui::InputText("Filename", saveFilename, 256);

	if (ImGui::Button("Save Level"))
	{
		SaveLevel();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Level"))
	{
		LoadLevel();
	}

	ImGui::Separator();

	// New Level creation
	ImGui::Text("Create New Level:");
	static int newWidth = 40;
	static int newHeight = 23;
	ImGui::InputInt("Width", &newWidth);
	ImGui::InputInt("Height", &newHeight);
	if (ImGui::Button("Create New"))
	{
		CreateNewLevel(newWidth, newHeight);
	}

	ImGui::Separator();
	ImGui::Text("Current Level: %dx%d", level->MAPWIDTH, level->MAPHEIGHT);

	ImGui::End();

}

void LevelEditor::Draw()
{
	DrawGrid();

	if (currentTileX >= 0 && currentTileX < level->MAPWIDTH && currentTileY >= 0 && currentTileY < level->MAPHEIGHT)
	{
		PlaceTilePreview(currentTileX, currentTileY);
	}
}

void LevelEditor::DrawGrid()
{
	if (!showGrid) return;

	// Draw vertical lines
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 64); // White with transparency
	for (int x = 0; x <= level->MAPWIDTH; x++)
	{
		SDL_RenderLine(renderer, (float)(x * Level::TILE_SIZE), 0.0f, (float)(x * Level::TILE_SIZE), (float)(level->MAPHEIGHT * Level::TILE_SIZE));
	}

	// Draw horizontal lines
	for (int y = 0; y <= level->MAPHEIGHT; y++)
	{
		SDL_RenderLine(renderer, 0.0f, (float)(y * Level::TILE_SIZE), (float)(level->MAPWIDTH * Level::TILE_SIZE), (float)(y * Level::TILE_SIZE));
	}
}

void LevelEditor::PlaceTilePreview(int x, int y)
{
	if (selectedTile == Level::TILE_AIR)
	{
		return;
	}

	SDL_Texture* texture = level->GetTileTexture(selectedTile);

	if (texture)
	{
		SDL_SetTextureAlphaMod(texture, 128);

		SDL_FRect Rect =
		{
			(float)(x * Level::TILE_SIZE),
			(float)(y * Level::TILE_SIZE),
			(float)Level::TILE_SIZE,
			(float)Level::TILE_SIZE
		};

		SDL_RenderTexture(renderer, texture, nullptr, &Rect);
		SDL_SetTextureAlphaMod(texture, 255);
	}
}

void LevelEditor::PlaceTile(int x, int y)
{
	// Check bounds
	if (x >= 0 && x < level->MAPWIDTH && y >= 0 && y < level->MAPHEIGHT)
	{
		level->Tiles.Get(x, y) = selectedTile;
		SDL_Log("Placed tile %d at (%d, %d)", selectedTile, x, y);
	}
}

void LevelEditor::SaveLevel()
{
	if (level->SaveToFile(saveFilename))
	{
		SDL_Log("Level saved to %s", saveFilename);
	}
	else
	{
		SDL_Log("Failed to save level!");
	}
}

void LevelEditor::LoadLevel()
{
	if (level->LoadFromFile(saveFilename))
	{
		level->LoadTextures(renderer); // Reload textures
		SDL_Log("Level loaded from %s", saveFilename);
	}
	else
	{
		SDL_Log("Failed to load level!");
	}
}

void LevelEditor::CreateNewLevel(int width, int height)
{
	if (width > 0 && height > 0 && width <= 1000 && height <= 1000)
	{
		level->MAPWIDTH = width;
		level->MAPHEIGHT = height;
		level->Tiles = Array2D<uint8_t>(width, height);

		// Initialize all tiles to AIR
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				level->Tiles.Get(x, y) = Level::TILE_AIR;
			}
		}

		SDL_Log("Created new level: %dx%d", width, height);
	}
}