#include "LevelEditor.h"
#include "imgui/imgui.h"

// Level Editor imgui input
void LevelEditor::HandleInput(SDL_Event& event)
{
	ImGuiIO& io = ImGui::GetIO();

	// Mouse wheel zoom
	if (event.type == SDL_EVENT_MOUSE_WHEEL)
	{
		float zoomDelta = event.wheel.y * 0.1f;
		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		float worldX, worldY;
		ScreenToWorld(mouseX, mouseY, worldX, worldY);

		cameraZoom += zoomDelta;
		cameraZoom = SDL_clamp(cameraZoom, MIN_ZOOM, MAX_ZOOM);

		cameraX = worldX - (mouseX / cameraZoom);
		cameraY = worldY - (mouseY / cameraZoom);
	}

	// Track mouse position
	if (event.type == SDL_EVENT_MOUSE_MOTION)
	{
		if (!io.WantCaptureMouse)
		{
			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			float worldX, worldY;
			ScreenToWorld(mouseX, mouseY, worldX, worldY);

			currentTileX = (int)(worldX / Level::TILE_SIZE);
			currentTileY = (int)(worldY / Level::TILE_SIZE);

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

			float worldX, worldY;
			ScreenToWorld(mouseX, mouseY, worldX, worldY);

			int tileX = (int)(worldX / Level::TILE_SIZE);
			int tileY = (int)(worldY / Level::TILE_SIZE);

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

	ImGui::BeginChild("TilePalette", ImVec2(0, 100), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4)); // Space between buttons
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // remove button padding

	const int tilesPerRow = 8;
	const float buttonSize = 32.0f;

	// Loop through All possible tiles
	int tileCount = 0;
	for (int i = 0; i < 256; i++)
	{
		// Skip if no texture
		if (i != Level::TILE_AIR && level->GetTileTexture(i) == nullptr)
		{
			continue;
		}

		ImGui::PushID(i);

		if (i == Level::TILE_AIR)
		{
			if (ImGui::Button("Air", ImVec2(buttonSize, buttonSize)))
			{
				selectedTile = i;
			}
		}
		else
		{
			if (ImGui::ImageButton("", (ImTextureID)(intptr_t)level->GetTileTexture(i), ImVec2(buttonSize, buttonSize)))
			{
				selectedTile = i;
			}
		}
		// Highlight selected
		if (selectedTile == i)
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			drawList->AddRect(
				ImVec2(min.x - 2, min.y - 2),
				ImVec2(max.x + 2, max.y + 2),
				IM_COL32(255, 255, 0, 255), 0.0f, 0, 3.0f);
		}

		ImGui::PopID();

		// Grid Layout
		tileCount++;
		if (tileCount % tilesPerRow != 0)
		{
			ImGui::SameLine();
		}
	}
	ImGui::PopStyleVar(2); // Pop both style vars
	ImGui::EndChild();

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
	static int newWidth = Level::MIN_WIDTH;
	static int newHeight = Level::MIN_HEIGHT;
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

	DrawLevelBoundary();

	if (currentTileX >= 0 && currentTileX < level->MAPWIDTH && currentTileY >= 0 && currentTileY < level->MAPHEIGHT)
	{
		PlaceTilePreview(currentTileX, currentTileY);
	}
}

void LevelEditor::DrawLevelBoundary()
{
	// Red boundary line
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	// Calculate screen coordinates of level bounds
	float left = (0 - cameraX) * cameraZoom;
	float top = (0 - cameraY) * cameraZoom;
	float right = (level->MAPWIDTH * Level::TILE_SIZE - cameraX) * cameraZoom;
	float bottom = (level->MAPHEIGHT * Level::TILE_SIZE - cameraY) * cameraZoom;

	// Draw rectangle
	SDL_RenderLine(renderer, left, top, right, top);
	SDL_RenderLine(renderer, right, top, right, bottom);
	SDL_RenderLine(renderer, right, bottom, left, bottom);
	SDL_RenderLine(renderer, left, bottom, left, top);
}

void LevelEditor::DrawGrid()
{
	if (!showGrid) return;

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 64); // White with transparency

	// Calculate visible grid range (same as RenderWithCamera)
	int startX = (int)(cameraX / Level::TILE_SIZE) - 1;
	int startY = (int)(cameraY / Level::TILE_SIZE) - 1;
	int endX = startX + (int)(windowWidth / (Level::TILE_SIZE * cameraZoom)) + 3;
	int endY = startY + (int)(windowHeight / (Level::TILE_SIZE * cameraZoom)) + 3;

	// Clamp to grid bounds (0 to Level size + 1 for border)
	startX = SDL_max(0, startX);
	startY = SDL_max(0, startY);
	endX = SDL_min(level->MAPWIDTH, endX);
	endY = SDL_min(level->MAPHEIGHT, endY);

	// Draw vertical lines
	for (int x = startX; x <= endX && x <= level->MAPWIDTH; x++)
	{
		float screenX = (x * Level::TILE_SIZE - cameraX) * cameraZoom;
		SDL_RenderLine(
			renderer,
			screenX,
			SDL_max(0.0f, (startY * Level::TILE_SIZE - cameraY) * cameraZoom),
			screenX,
			SDL_min((float)windowHeight, (endY * Level::TILE_SIZE - cameraY) * cameraZoom)
		);
	}

	// Draw horizontal lines
	for (int y = startY; y <= endY && y <= level->MAPHEIGHT; y++)
	{
		float screenY = (y * Level::TILE_SIZE - cameraY) * cameraZoom;
		SDL_RenderLine(
			renderer,
			SDL_max(0.0f, (startX * Level::TILE_SIZE - cameraX) * cameraZoom),
			screenY,
			SDL_min((float)windowWidth, (endX * Level::TILE_SIZE - cameraX) * cameraZoom),
			screenY
		);
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

		// Transform to camera
		SDL_FRect screenRect;
		screenRect.x = (x * Level::TILE_SIZE - cameraX) * cameraZoom;
		screenRect.y = (y * Level::TILE_SIZE - cameraY) * cameraZoom;
		screenRect.w = Level::TILE_SIZE * cameraZoom;
		screenRect.h = Level::TILE_SIZE * cameraZoom;


		SDL_RenderTexture(renderer, texture, nullptr, &screenRect);
		SDL_SetTextureAlphaMod(texture, 255);
	}
}

void LevelEditor::PlaceTile(int x, int y)
{
	// Check bounds
	if (x >= 0 && x < level->MAPWIDTH && y >= 0 && y < level->MAPHEIGHT)
	{
		level->Tiles.Get(x, y) = selectedTile;
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

	ResetZoom();
	RecenterCamera();
}

void LevelEditor::CreateNewLevel(int width, int height)
{
	// Check selected level size before creating level
	if (width >= Level::MIN_WIDTH && height >= Level::MIN_HEIGHT && width <= Level::MAX_WIDTH && height <= Level::MAX_HEIGHT)
	{
		level->MAPWIDTH = width;
		level->MAPHEIGHT = height;

		// Resize the current level (We dont have to reload textures this way)
		level->Tiles.Resize(width, height);

		// Clear all tiles to Air = 0
		level->Tiles.Clear(Level::TILE_AIR);

		SDL_Log("Created new level: %dx%d", width, height);
	}
	else
	{
		SDL_Log("Invalid level size! Must be between %dx%d and %dx%d", Level::MIN_WIDTH, Level::MIN_HEIGHT, Level::MAX_WIDTH, Level::MAX_HEIGHT);
	}

	ResetZoom();
	RecenterCamera();
}

void LevelEditor::RecenterCamera()
{
	// Center camera
	cameraX = (level->MAPWIDTH * Level::TILE_SIZE) / 2.0f - (windowWidth / 2.0f / cameraZoom);
	cameraY = (level->MAPHEIGHT * Level::TILE_SIZE) / 2.0f - (windowHeight / 2.0f / cameraZoom);
}

void LevelEditor::ResetZoom()
{
	cameraZoom = DEFAULT_ZOOM;
}

void LevelEditor::ScreenToWorld(float screenX, float screenY, float& worldX, float& worldY)
{
	worldX = (screenX / cameraZoom) + cameraX;
	worldY = (screenY / cameraZoom) + cameraY;
}

void LevelEditor::UpdateCamera(float deltaTime)
{
	
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard || io.WantCaptureMouse)
	{
		return;
	}

	const bool* keys = SDL_GetKeyboardState(NULL);
	float moveSpeed = 300.0f * deltaTime / cameraZoom; // Movement scaled with zoom

	// Keyboard movement (wasd)
	if (keys[SDL_SCANCODE_W]) 
	{
		cameraY -= moveSpeed;
	}
	if (keys[SDL_SCANCODE_S])
	{
		cameraY += moveSpeed;
	}
	if (keys[SDL_SCANCODE_A])
	{
		cameraX -= moveSpeed;
	}
	if (keys[SDL_SCANCODE_D])
	{
		cameraX += moveSpeed;
	}

	// Edge scrolling
	float mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mouseX < EDGE_SCROLL_MARGIN)
	{
		cameraX -= EDGE_SCROLL_SPEED * deltaTime / cameraZoom;
	}
	if (mouseX > windowWidth - EDGE_SCROLL_MARGIN)
	{
		cameraX += EDGE_SCROLL_SPEED * deltaTime / cameraZoom;
	}
	if (mouseY < EDGE_SCROLL_MARGIN)
	{
		cameraY -= EDGE_SCROLL_SPEED * deltaTime / cameraZoom;
	}
	if (mouseY > windowHeight - EDGE_SCROLL_MARGIN)
	{
		cameraY += EDGE_SCROLL_SPEED * deltaTime / cameraZoom;
	}

	// Camera bounds (25 tiles beyond level edges
	float boundBuffer = 25 * Level::TILE_SIZE;
	float levelWidth = level->MAPWIDTH * Level::TILE_SIZE;
	float levelHeight = level->MAPHEIGHT * Level::TILE_SIZE;

	// Calculate visible area based on zoom
	float visibleWidth = windowWidth / cameraZoom;
	float visibleHeight = windowHeight / cameraZoom;

	// Check if Level fits within window
	if (levelWidth + 2 * boundBuffer <= visibleWidth)
	{
		// Level fits horizontally - center it
		cameraX = (levelWidth - visibleWidth) / 2.0f;
	}
	else
	{
		// Level does not fit - allow scrolling
		float minX = -boundBuffer;
		float maxX = levelWidth - visibleWidth + boundBuffer;
		cameraX = SDL_clamp(cameraX, minX, maxX);
	}

	if (levelHeight + 2 * boundBuffer <= visibleHeight)
	{
		// Level fits vertically - center it
		cameraY = (levelHeight - visibleHeight) / 2.0f;
	}
	else
	{
		float minY = -boundBuffer;
		float maxY = levelHeight - visibleHeight + boundBuffer;
		cameraY = SDL_clamp(cameraY, minY, maxY);
	}
}

void LevelEditor::Update()
{
	// Calculate deltaTime
	Uint64 currentTime = SDL_GetTicks();
	deltaTime = (currentTime - lastFrameTime) / 1000.0f;
	lastFrameTime = currentTime;

	// Update camera with the calculated deltaTime
	UpdateCamera(deltaTime);
}

void LevelEditor::RenderWithCamera()
{
	// Visible tile range with 1 tile buffer for smooth scrolling
	int startX = (int)(cameraX / Level::TILE_SIZE) - 1;
	int startY = (int)(cameraY / Level::TILE_SIZE) - 1;
	int endX = startX + (int)(windowWidth / (Level::TILE_SIZE * cameraZoom)) + 3;
	int endY = startY + (int)(windowHeight / (Level::TILE_SIZE * cameraZoom)) + 3;

	// Clamp to Level bounds
	startX = SDL_max(0, startX);
	startY = SDL_max(0, startY);
	endX = SDL_min(level->MAPWIDTH, endX);
	endY = SDL_min(level->MAPHEIGHT, endY);

	// Only render visible tiles
	for (int y = startY; y < endY; y++)
	{
		for (int x = startX; x < endX; x++)
		{

			uint8_t tileType = level->Tiles.Get(x, y);

			if (tileType == Level::TILE_AIR || level->tileTextures[tileType] == nullptr)
			{
				continue;
			}

			// Calculate screen position with camera transform
			SDL_FRect screenRect;
			screenRect.x = (x * Level::TILE_SIZE - cameraX) * cameraZoom;
			screenRect.y = (y * Level::TILE_SIZE - cameraY) * cameraZoom;
			screenRect.w = Level::TILE_SIZE * cameraZoom;
			screenRect.h = Level::TILE_SIZE * cameraZoom;

			SDL_RenderTexture(renderer, level->GetTileTexture(tileType), nullptr, &screenRect);

		}
	}
}