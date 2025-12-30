#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleUI.h"
#include "ModuleRender.h"
#include "ModuleState.h"
#include "ModuleGame.h"
#include <math.h>
#include <string>
#include <algorithm>

#include "raylib.h"

ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

// Destructor
ModuleUI::~ModuleUI()
{
}

bool ModuleUI::Init()
{
	LOG("Creating Renderer context");
	bool ret = true;
	return ret;
}

// PreUpdate
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

// PostUpdate
update_status ModuleUI::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleUI::CleanUp()
{
	return true;
}

// ------------------------------------------------------------
// BUTTON
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

	Color bg = hover ? bgHover : bgNormal;

	DrawRectangleRec(rect, bg);
	DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, border);

	if (hover)
		DrawRectangleLinesEx(rect, 2.0f, YELLOW);

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

	return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ------------------------------------------------------------
// MAIN MENU (CENTERED BLOCK)
// ------------------------------------------------------------
void ModuleUI::UpdateMainMenu()
{
	const int buttonW = 260;
	const int buttonH = 64;
	const int spacing = 18;

	const int cx = SCREEN_WIDTH / 2 - buttonW / 2;
	const int centerY = SCREEN_HEIGHT / 2;

	if (Button(cx, centerY - buttonH - spacing / 2, buttonW, buttonH, "JUGAR"))
	{
		App->state->ChangeState(GameState::MENU_PLAY);
	}

	if (Button(cx, centerY + spacing / 2, buttonW, buttonH, "CONFIG"))
	{
		App->state->ChangeState(GameState::MENU_OPTIONS);
	}
}

// ------------------------------------------------------------
// PLAY MENU
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

	int _mapId = 1;

	for (int row = 0; row < 2; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			int x = startX + col * (buttonW + spacingX);
			int y = startY + row * (buttonH + spacingY);

			if (Button(x, y, buttonW, buttonH, ""))
			{
				App->state->mapId = _mapId;
				App->state->ChangeState(GameState::RACE);
			}
			_mapId++;
		}
	}

	const int backW = 240;
	const int backH = 60;

	if (Button(centerX - backW / 2, SCREEN_HEIGHT - 120, backW, backH, "ATRAS"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}

// ------------------------------------------------------------
// OPTIONS MENU
// ------------------------------------------------------------
void ModuleUI::UpdateOptionsMenu()
{
	const int buttonW = 240;
	const int buttonH = 60;

	if (Button(SCREEN_WIDTH / 2 - buttonW / 2, SCREEN_HEIGHT / 2, buttonW, buttonH, "ATRAS"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}

// ------------------------------------------------------------
// RACE UI
// ------------------------------------------------------------
void ModuleUI::UpdateRaceUI()
{
	const std::vector<int>& leaderboard = App->scene_intro->leaderboard;

	int x = 20;
	int y = 20;

	DrawText("POSICIONES", x, y, 20, WHITE);
	y += 30;

	for (int i = 0; i < leaderboard.size(); ++i)
	{
		std::string text = std::to_string(i + 1) + " Coche " + std::to_string(leaderboard[i]);
		DrawText(text.c_str(), x, y, 18, WHITE);
		y += 22;
	}
}

// ------------------------------------------------------------
// LEADERBOARD
// ------------------------------------------------------------
void ModuleUI::SyncLeaderboard()
{
	const std::vector<int>& data = App->scene_intro->leaderboard;

	if (leaderboardUI.size() != data.size())
	{
		leaderboardUI.clear();

		for (int i = 0; i < data.size(); ++i)
		{
			leaderboardUI.push_back({
				data[i],
				20.0f + i * 24.0f,
				20.0f + i * 24.0f,
				8.0f
				});
		}
	}

	for (int i = 0; i < data.size(); ++i)
	{
		int id = data[i];

		auto it = std::find_if(leaderboardUI.begin(), leaderboardUI.end(),
			[&](const LeaderboardEntryUI& e) { return e.carId == id; });

		if (it != leaderboardUI.end())
		{
			it->targetY = 50.0f + i * 24.0f;
			it->lastRank = it->rank;
			it->rank = i + 1;

			if (it->lastRank != -1 && it->lastRank != it->rank)
				it->flashTimer = 0.5f;
		}
	}
}

void ModuleUI::UpdateLeaderboardAnimation()
{
	float dt = App->deltaTime;

	for (auto& entry : leaderboardUI)
	{
		float diff = entry.targetY - entry.y;
		entry.y += diff * entry.animSpeed * dt;
	}
}

void ModuleUI::DrawLeaderboard()
{
	int x = 20;
	int yStart = 50;
	int spacing = 24;

	DrawText("POSICIONES", x, 20, 20, BLACK);

	for (int i = 0; i < leaderboardUI.size(); ++i)
	{
		auto& e = leaderboardUI[i];

		DrawText(std::to_string(i + 1).c_str(), x, yStart + i * spacing, 18, BLACK);

		Color c = (e.carId == 0) ? YELLOW : BLACK;

		if (e.flashTimer > 0)
		{
			c = (e.lastRank > i) ? GREEN : RED;
			e.flashTimer -= App->deltaTime;
		}

		std::string text = " Coche " + std::to_string(e.carId);
		DrawText(text.c_str(), x + 30, (int)e.y, 18, c);
	}
}

// ------------------------------------------------------------
// RESULTS
// ------------------------------------------------------------
void ModuleUI::DrawResultsScreen()
{
	int cx = SCREEN_WIDTH / 2;
	int y = 80;

	DrawText("RACE RESULTS", cx - 100, y, 30, BLACK);
	y += 50;

	DrawText("FINAL POSITIONS", 80, y, 22, BLACK);
	y += 30;

	const auto& leaderboard = App->scene_intro->results.finalLeaderboard;

	for (int i = 0; i < leaderboard.size(); ++i)
	{
		std::string line = std::to_string(i + 1) + ". Car " + std::to_string(leaderboard[i]);
		DrawText(line.c_str(), 80, y, 18, BLACK);
		y += 22;
	}

	y += 20;
	DrawText("LAP TIMES", 80, y, 22, BLACK);
	y += 30;

	const auto& laps = App->scene_intro->results.lapTimes;

	for (int i = 0; i < laps.size(); ++i)
	{
		char buffer[64];
		sprintf_s(buffer, "Lap %d: %.2f s", i + 1, laps[i]);
		DrawText(buffer, 80, y, 18, BLACK);
		y += 22;
	}

	y += 20;
	char total[64];
	sprintf_s(total, "Total Time: %.2f s", App->scene_intro->results.totalTime);
	DrawText(total, 80, y, 22, DARKBLUE);

	if (Button(cx - 120, SCREEN_HEIGHT - 120, 240, 60, "VOLVER AL MENU"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}

// ------------------------------------------------------------
// TIMER
// ------------------------------------------------------------
void ModuleUI::DrawRaceTimer()
{
	double time = App->scene_intro->GetRaceTime();

	int minutes = (int)(time / 60.0);
	int seconds = (int)time % 60;
	int millis = (int)((time - (int)time) * 1000);

	char buffer[32];
	sprintf_s(buffer, "%02d:%02d.%03d", minutes, seconds, millis);

	DrawText(buffer, SCREEN_WIDTH / 2 - 80, 20, 30, BLACK);
}
