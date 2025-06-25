#include "LevelEditor.h"
#include "imgui/imgui.h"

// Level Editor imgui input
void LevelEditor::HandleInput(SDL_Event& event)
{
	ImGuiIO& io = ImGui::GetIO();

	// Mouse wheel zoom
	if (event.type == SDL_EVENT_MOUSE_WHEEL && !isPlaying)
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
			if (isDrawing && currentMode == MODE_TILES)
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

			float mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			float worldX, worldY;
			ScreenToWorld(mouseX, mouseY, worldX, worldY);

			if (currentMode == MODE_TILES)
			{
				isDrawing = true;
				int tileX = (int)(worldX / Level::TILE_SIZE);
				int tileY = (int)(worldY / Level::TILE_SIZE);
				PlaceTile(tileX, tileY);
				lastPlacedX = tileX;
				lastPlacedY = tileY;
			}
			else if (currentMode == MODE_PLAYER_SPAWN)
			{
				// Place spawn at exact click position
				level->playerSpawnPoint = Vec2(worldX, worldY);
				level->hasPlayerSpawn = true;
				SDL_Log("Player spawn set to (%.0f, %.0f)", worldX, worldY);
			}
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

	ImGui::Separator();
	ImGui::Text("Editor Mode:");

	if (!isPlaying)
	{
		if (ImGui::RadioButton("Place TIles", currentMode == MODE_TILES))
		{
			currentMode = MODE_TILES;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Set Player Spawn", currentMode == MODE_PLAYER_SPAWN))
		{
			currentMode = MODE_PLAYER_SPAWN;
		}
	}
	else
	{
		ImGui::Text("Currently in Play Mode");
	}

	// Play/Stop button
	ImGui::Separator();
	if (!isPlaying)
	{
		if (ImGui::Button("PLAY", ImVec2(100, 30)))
		{
			StartPlayMode();
		}
	}
	else
	{
		if (ImGui::Button("STOP", ImVec2(100, 30)))
	    {
		StopPlayMode();
	    }
	}

	// Show spawn info when in spawn mode
	if (currentMode == MODE_PLAYER_SPAWN)
	{
		ImGui::Text("Click to place player spawn point");
		if (level->hasPlayerSpawn)
		{
			ImGui::Text("Current spawn: (%.0f, %.0f)",
				level->playerSpawnPoint.x,
				level->playerSpawnPoint.y);
		}
	}

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

	DrawPlayerSpawn();

	if (isPlaying)
	{
		DrawPlayer();
	}

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

void LevelEditor::DrawPlayerSpawn()
{
	if (!level->hasPlayerSpawn)
	{
		return;
	}

	// Convert world position to screen position
	float screenX = (level->playerSpawnPoint.x - cameraX) * cameraZoom;
	float screenY = (level->playerSpawnPoint.y - cameraY) * cameraZoom;

	if (playerSpawnIcon)
	{
		// Draw icon matching player size
		SDL_FRect destRect;
		destRect.w = 12 * cameraZoom;
		destRect.h = 24 * cameraZoom;
		destRect.x = screenX - destRect.w / 2;
		destRect.y = screenY - destRect.h / 2;

		SDL_RenderTexture(renderer, playerSpawnIcon, nullptr, &destRect);
	}
	else
	{
		// Fallback Draw a yellow rectangle
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

		SDL_FRect playerRect;
		playerRect.w = 12 * cameraZoom;
		playerRect.h = 24 * cameraZoom;
		playerRect.x = screenX - playerRect.w / 2;
		playerRect.y = screenY - playerRect.h / 2;

		SDL_RenderRect(renderer, &playerRect);

		// Draw cross in center for exact position
		SDL_RenderLine(renderer, screenX - 5, screenY, screenX + 5, screenY);
		SDL_RenderLine(renderer, screenX, screenY - 5, screenX, screenY + 5);
	}

}

void LevelEditor::DrawPlayer()
{
	if (!world || playerEntity == NULL_ENTITY)
	{
		return;
	}

	Transform* transform = world->GetTransform(playerEntity);
	Sprite* sprite = world->GetSprite(playerEntity);

	if (transform && sprite && sprite->texture)
	{
		SDL_FRect destRect;
		destRect.w = sprite->width * cameraZoom;
		destRect.h = sprite->height * cameraZoom;
		destRect.x = (transform->position.x - cameraX) * cameraZoom - destRect.w / 2;
		destRect.y = (transform->position.y - cameraY) * cameraZoom - destRect.h / 2;

		SDL_RenderTexture(renderer, sprite->texture, nullptr, &destRect);
	}
	else if (transform)
	{
		// Fallback: Draw green rectangele
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

		SDL_FRect playerRect;
		playerRect.w = 12 * cameraZoom;
		playerRect.h = 24 * cameraZoom;
		playerRect.x = (transform->position.x - cameraX) * cameraZoom - playerRect.w / 2;
		playerRect.y = (transform->position.y - cameraY) * cameraZoom - playerRect.h / 2;

		SDL_RenderRect(renderer, &playerRect);
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

float LevelEditor::GetPlayModeZoom() const
{
	return windowWidth / 640.0f;
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

	if (!isPlaying)
	{
		const bool* keys = SDL_GetKeyboardState(NULL);
		float moveSpeed = 300.0f * deltaTime / cameraZoom; // Movement scaled with zoom

		// Keyboard movement (wasd) only in edit mode
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

		// Edge scrolling only in edit mode
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
	}
	else
	{
		// PLAY MODE: Camera follow the player now
		if (world && playerEntity != NULL_ENTITY)
		{
			Transform* playerTransform = world->GetTransform(playerEntity);
			if (playerTransform)
			{
				// Center camera on player
				Vec2 targetCameraPos;
				targetCameraPos.x = playerTransform->position.x - (windowWidth / 2.0f / cameraZoom);
				targetCameraPos.y = playerTransform->position.y - (windowHeight / 2.0f / cameraZoom);

				// Smooth camera following
				float lerpFactor = 5.0f * deltaTime;
				cameraX += (targetCameraPos.x - cameraX) * lerpFactor;
				cameraY += (targetCameraPos.y - cameraY) * lerpFactor;
			}
		}
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

void LevelEditor::UpdateWorld(float dt)
{
	if (!isPlaying || !world)
	{
		return;
	}

	// Update player controller first (handles input)
	if (playerControllerSystem)
	{
		playerControllerSystem->Update(*world, dt);
	}

	if (physicsSystem)
	{
		physicsSystem->Update(*world, *level, dt);
	}
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

void LevelEditor::StartPlayMode()
{
	if (!level->hasPlayerSpawn)
	{
		SDL_Log("Cannot play: No player spawn point set!");
		return;
	}

	isPlaying = true;
	currentMode = MODE_PLAY;
	SDL_Log("Entering play mode");

	cameraZoom = GetPlayModeZoom();

	// Create the ECS world
	world = new World();

	// Create player entity
	playerEntity = world->CreateEntity();

	// Add transform
	world->AddTransform(playerEntity, Transform{
		level->playerSpawnPoint,
		0.0f,
		1.0f
		});

	// Add physics
	world->AddPhysics(playerEntity, Physics{
		Vec2(0,0),   // Velocity
		Vec2(0,0),   // Acceleration
		1.0f,        // gravityScale
		600.0f,      // maxFallSpeed
		0.0f,        // linearDamping
		false        // isKinematic
		});

	// Add collider
	world->AddCollider(playerEntity, Collider{
		Vec2(12.0f, 24.0f),   // Size
		Vec2(0.0f, 0.0f),     // Offset
		LAYER_PLAYER,         // Layer
		0xFFFF,               // Collides with everything
		false,                // isTrigger
		false                 // isStatic
		});

	// Add collision state
	world->AddCollisionState(playerEntity, CollisionState{});

	// Add player component
	world->AddPlayer(playerEntity, Player{});

	// Load and add sprite
	SDL_Texture* playerTexture = IMG_LoadTexture(renderer, "Assets/player.png");
	if (playerTexture)
	{
		world->AddSprite(playerEntity, Sprite{
			playerTexture,
			SDL_Color{255,255,255,255},
			12,      // Width
			24       // Height
			});
	}
	else
	{
		SDL_Log("Warning: No player texture found at Assets/player.png");
	}

	SDL_Log("Player entity created at (%.0f, %.0f)",
		level->playerSpawnPoint.x,
		level->playerSpawnPoint.y);

	// Initialize systems
	physicsSystem = new PhysicsSystem(600.0f);
	playerControllerSystem = new PlayerControllerSystem();
}

void LevelEditor::StopPlayMode()
{
	isPlaying = false;
	currentMode = MODE_TILES;
	SDL_Log("Exiting play mode");

	ResetZoom();
	RecenterCamera();

	// Clean up systems
	if (physicsSystem)
	{
		delete physicsSystem;
		physicsSystem = nullptr;
	}

	if (playerControllerSystem)
	{
		delete playerControllerSystem;
		playerControllerSystem = nullptr;
	}

	if (world)
	{
		delete world;
		world = nullptr;
	}

	playerEntity = NULL_ENTITY;
}