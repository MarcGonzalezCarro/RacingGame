#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleUI.h"
#include "ModuleRender.h"
#include "ModuleState.h"
#include "ModuleGame.h"
#include "ModuleAudio.h"

#include <math.h>
#include <string>
#include <algorithm>

#include "raylib.h"

ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleUI::~ModuleUI()
{
}

bool ModuleUI::Init()
{
	LOG("Initializing UI module");

	// Load UI button press sound once
	pressFx = App->audio->LoadFx("Assets/Audio/SFX/pressSound.wav");

	return true;
}

update_status ModuleUI::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleUI::Update()
{
	switch (App->state->GetState())
	{
	case GameState::MENU_MAIN:
		UpdateMainMenu();
		break;

	case GameState::MENU_PLAY:
		UpdatePlayMenu();
		break;

	case GameState::MENU_OPTIONS:
		UpdateOptionsMenu();
		break;

	case GameState::RACE:
		SyncLeaderboard();
		UpdateLeaderboardAnimation();
		DrawLeaderboard();
		DrawRaceTimer();
		break;

	case GameState::RESULTS:
		DrawResultsScreen();
		break;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModuleUI::CleanUp()
{
	return true;
}

// ------------------------------------------------------------
// Generic button rendering and interaction
// ------------------------------------------------------------
bool ModuleUI::Button(int x, int y, int w, int h, const char* text)
{
	Vector2 mouse = { (float)GetMouseX(), (float)GetMouseY() };
	Rectangle rect = { (float)x, (float)y, (float)w, (float)h };

	bool hover = CheckCollisionPointRec(mouse, rect);

	Color bgNormal = { 90, 90, 90, 255 };
	Color bgHover = { 120, 120, 120, 255 };
	Color border = BLACK;
	Color textCol = RAYWHITE;

	DrawRectangleRec(rect, hover ? bgHover : bgNormal);
	DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, border);

	// Highlight when hovered
	if (hover)
	{
		DrawRectangleLinesEx(rect, 2.0f, YELLOW);
	}

	// Draw centered text if provided
	if (text && text[0] != '\0')
	{
		Font font = App->renderer->fuente;

		float fontSize = (float)h * 0.45f;
		if (fontSize < 18.0f) fontSize = 18.0f;
		if (fontSize > 28.0f) fontSize = 28.0f;

		const float spacing = 1.0f;

		Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);

		float textX = roundf(rect.x + (rect.width - textSize.x) * 0.5f);
		float textY = roundf(rect.y + (rect.height - textSize.y) * 0.5f);

		DrawTextEx(font, text, { textX, textY }, fontSize, spacing, textCol);
	}

	// Handle click
	if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		App->audio->PlayFx(pressFx);
		return true;
	}

	return false;
}

// ------------------------------------------------------------
// Main menu
// ------------------------------------------------------------
void ModuleUI::UpdateMainMenu()
{
	const int buttonW = 260;
	const int buttonH = 64;
	const int spacing = 18;

	int x = SCREEN_WIDTH / 2 - buttonW / 2;
	int centerY = SCREEN_HEIGHT / 2;

	if (Button(x, centerY - buttonH - spacing / 2, buttonW, buttonH, "JUGAR"))
	{
		App->state->ChangeState(GameState::MENU_PLAY);
	}

	if (Button(x, centerY + spacing / 2, buttonW, buttonH, "CONFIG"))
	{
		App->state->ChangeState(GameState::MENU_OPTIONS);
	}
}

// ------------------------------------------------------------
// Map selection menu
// ------------------------------------------------------------
void ModuleUI::UpdatePlayMenu()
{
	int centerX = SCREEN_WIDTH / 2;

	DrawText(
		"SELECCIONA MAPA",
		centerX - MeasureText("SELECCIONA MAPA", 24) / 2,
		120,
		24,
		BLACK
	);

	const int buttonW = 160;
	const int buttonH = 50;
	const int spacingX = 20;
	const int spacingY = 20;

	int startX = centerX - (buttonW * 3 + spacingX * 2) / 2;
	int startY = 180;

	int mapId = 1;

	for (int row = 0; row < 2; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			if (Button(
				startX + col * (buttonW + spacingX),
				startY + row * (buttonH + spacingY),
				buttonW,
				buttonH,
				""))
			{
				App->state->mapId = mapId;
				App->state->ChangeState(GameState::RACE);
			}
			mapId++;
		}
	}

	if (Button(centerX - 120, SCREEN_HEIGHT - 120, 240, 60, "ATRAS"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}

// ------------------------------------------------------------
// Options menu
// ------------------------------------------------------------
void ModuleUI::UpdateOptionsMenu()
{
	if (Button(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2, 240, 60, "ATRAS"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}

// ------------------------------------------------------------
// Leaderboard logic and rendering
// ------------------------------------------------------------
void ModuleUI::SyncLeaderboard()
{
	const auto& data = App->scene_intro->leaderboard;

	if (leaderboardUI.size() != data.size())
	{
		leaderboardUI.clear();

		for (int i = 0; i < data.size(); ++i)
		{
			leaderboardUI.push_back({ data[i], 20.0f, 20.0f, 8.0f });
		}
	}

	for (int i = 0; i < data.size(); ++i)
	{
		for (auto& e : leaderboardUI)
		{
			if (e.carId == data[i])
			{
				e.targetY = 50.0f + i * 24.0f;
				e.lastRank = e.rank;
				e.rank = i + 1;

				if (e.lastRank != -1 && e.lastRank != e.rank)
					e.flashTimer = 0.5f;
			}
		}
	}
}

void ModuleUI::UpdateLeaderboardAnimation()
{
	for (auto& e : leaderboardUI)
	{
		float diff = e.targetY - e.y;
		e.y += diff * e.animSpeed * App->deltaTime;
	}
}

void ModuleUI::DrawLeaderboard()
{
	DrawText("POSICIONES", 20, 20, 20, BLACK);

	for (int i = 0; i < leaderboardUI.size(); ++i)
	{
		auto& e = leaderboardUI[i];

		Color color = (e.carId == 0) ? YELLOW : BLACK;

		if (e.flashTimer > 0.0f)
		{
			color = (e.lastRank > i) ? GREEN : RED;
			e.flashTimer -= App->deltaTime;
		}

		DrawText(TextFormat("%d", i + 1), 20, 50 + i * 24, 18, BLACK);
		DrawText(TextFormat("Coche %d", e.carId), 50, (int)e.y, 18, color);
	}
}

// ------------------------------------------------------------
// Results screen and race timer
// ------------------------------------------------------------
void ModuleUI::DrawResultsScreen()
{
	int y = 80;

	DrawText("RACE RESULTS", SCREEN_WIDTH / 2 - 100, y, 30, BLACK);
	y += 50;

	DrawText("FINAL POSITIONS", 80, y, 22, BLACK);
	y += 30;

	for (int i = 0; i < App->scene_intro->results.finalLeaderboard.size(); ++i)
	{
		DrawText(
			TextFormat("%d. Car %d", i + 1, App->scene_intro->results.finalLeaderboard[i]),
			80,
			y,
			18,
			BLACK
		);
		y += 22;
	}

	if (Button(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT - 120, 240, 60, "VOLVER AL MENU"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}

void ModuleUI::DrawRaceTimer()
{
	double time = App->scene_intro->GetRaceTime();

	int minutes = (int)(time / 60.0);
	int seconds = (int)time % 60;
	int millis = (int)((time - (int)time) * 1000);

	DrawText(
		TextFormat("%02d:%02d.%03d", minutes, seconds, millis),
		SCREEN_WIDTH / 2 - 80,
		20,
		30,
		BLACK
	);
}
