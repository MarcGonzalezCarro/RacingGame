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

static float Clamp01(float v)
{
	if (v < 0.0f) return 0.0f;
	if (v > 1.0f) return 1.0f;
	return v;
}

static float Slider01(Rectangle bar, float value01, bool& dragging)
{
	Vector2 mouse = { (float)GetMouseX(), (float)GetMouseY() };
	bool hovered = CheckCollisionPointRec(mouse, bar);

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered)
		dragging = true;

	if (dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
	{
		float t = (mouse.x - bar.x) / bar.width;
		value01 = Clamp01(t);
	}

	if (dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
		dragging = false;

	DrawRectangleRec(bar, Color{ 70, 70, 70, 255 });
	DrawRectangleLines((int)bar.x, (int)bar.y, (int)bar.width, (int)bar.height, BLACK);

	Rectangle fill = bar;
	fill.width = bar.width * value01;
	DrawRectangleRec(fill, Color{ 120, 120, 120, 255 });

	float handleX = roundf(bar.x + bar.width * value01);
	Rectangle handle = { handleX - 6.0f, bar.y - 4.0f, 12.0f, bar.height + 8.0f };
	DrawRectangleRec(handle, Color{ 200, 200, 200, 255 });
	DrawRectangleLines((int)handle.x, (int)handle.y, (int)handle.width, (int)handle.height, BLACK);

	return value01;
}

ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleUI::~ModuleUI()
{
}

bool ModuleUI::Init()
{
	LOG("Initializing UI module");

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

bool ModuleUI::Button(int x, int y, int w, int h, const char* text)
{
	Vector2 mouse = { (float)GetMouseX(), (float)GetMouseY() };
	Rectangle rect = { (float)x, (float)y, (float)w, (float)h };

	bool hover = CheckCollisionPointRec(mouse, rect);

	Color bgNormal = Color{ 90, 90, 90, 255 };
	Color bgHover = Color{ 120, 120, 120, 255 };
	Color border = BLACK;
	Color textCol = RAYWHITE;

	DrawRectangleRec(rect, hover ? bgHover : bgNormal);
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

		DrawTextEx(font, text, Vector2{ textX, textY }, fontSize, spacing, textCol);
	}

	if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		App->audio->PlayFx(pressFx);
		return true;
	}

	return false;
}

void ModuleUI::UpdateMainMenu()
{
	const int buttonW = 260;
	const int buttonH = 64;
	const int spacing = 18;

	int x = SCREEN_WIDTH / 2 - buttonW / 2;
	int centerY = SCREEN_HEIGHT / 2;

	if (Button(x, centerY - buttonH - spacing / 2, buttonW, buttonH, "JUGAR"))
		App->state->ChangeState(GameState::MENU_PLAY);

	if (Button(x, centerY + spacing / 2, buttonW, buttonH, "CONFIG"))
		App->state->ChangeState(GameState::MENU_OPTIONS);
}

void ModuleUI::UpdatePlayMenu()
{
	int centerX = SCREEN_WIDTH / 2;

	DrawText("SELECCIONA MAPA",
		centerX - MeasureText("SELECCIONA MAPA", 24) / 2,
		120, 24, BLACK);

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
			if (Button(startX + col * (buttonW + spacingX),
				startY + row * (buttonH + spacingY),
				buttonW, buttonH, ""))
			{
				App->state->mapId = mapId;
				App->state->ChangeState(GameState::RACE);
			}
			mapId++;
		}
	}

	if (Button(centerX - 120, SCREEN_HEIGHT - 120, 240, 60, "ATRAS"))
		App->state->ChangeState(GameState::MENU_MAIN);
}

void ModuleUI::UpdateOptionsMenu()
{
	const int panelW = 520;
	const int panelH = 320;
	const int panelX = SCREEN_WIDTH / 2 - panelW / 2;
	const int panelY = SCREEN_HEIGHT / 2 - panelH / 2;

	DrawRectangle(panelX, panelY, panelW, panelH, Color{ 0, 0, 0, 120 });
	DrawRectangleLines(panelX, panelY, panelW, panelH, BLACK);

	DrawText("CONFIGURACION", panelX + 20, panelY + 20, 28, RAYWHITE);

	// SFX Volume
	{
		float sfx = App->audio->GetSfxVolume();

		DrawText("VOLUMEN SFX", panelX + 20, panelY + 80, 20, RAYWHITE);

		Rectangle bar = Rectangle{ (float)(panelX + 220), (float)(panelY + 82), 250.0f, 18.0f };
		sfx = Slider01(bar, sfx, draggingSfx);
		App->audio->SetSfxVolume(sfx);

		int percent = (int)roundf(sfx * 100.0f);
		DrawText(TextFormat("%d%%", percent), panelX + 480, panelY + 78, 20, RAYWHITE);
	}

	// Music Enabled Toggle
	{
		bool enabled = App->audio->IsMusicEnabled();
		const char* label = enabled ? "MUSICA: ON" : "MUSICA: OFF";

		if (Button(panelX + 20, panelY + 130, 200, 56, label))
		{
			bool newEnabled = !enabled;
			App->audio->SetMusicEnabled(newEnabled);

			// If music is enabled again, resume the appropriate track for the current context
			if (newEnabled)
			{
				GameState st = App->state->GetState();

				// Menus use menuSong
				if (st == GameState::MENU_MAIN || st == GameState::MENU_PLAY || st == GameState::MENU_OPTIONS)
				{
					App->audio->PlayMusic("Assets/Audio/SFX/menuSong.wav", 0.0f, true);
				}
				// Results uses endSong
				else if (st == GameState::RESULTS)
				{
					App->audio->PlayMusic("Assets/Audio/SFX/endSong.wav", 0.0f, false);
				}
				// Race: keep silent (by design)
			}
		}

	}

	// Music Volume
	{
		float mv = App->audio->GetMusicVolume();

		DrawText("VOLUMEN MUSICA", panelX + 20, panelY + 210, 20, RAYWHITE);

		Rectangle bar = Rectangle{ (float)(panelX + 220), (float)(panelY + 212), 250.0f, 18.0f };
		mv = Slider01(bar, mv, draggingMusic);
		App->audio->SetMusicVolume(mv);

		int percent = (int)roundf(mv * 100.0f);
		DrawText(TextFormat("%d%%", percent), panelX + 480, panelY + 208, 20, RAYWHITE);
	}

	if (Button(panelX + panelW - 260, panelY + panelH - 76, 240, 56, "ATRAS"))
		App->state->ChangeState(GameState::MENU_MAIN);
}

void ModuleUI::SyncLeaderboard()
{
	const auto& data = App->scene_intro->leaderboard;

	if (leaderboardUI.size() != data.size())
	{
		leaderboardUI.clear();

		for (int i = 0; i < (int)data.size(); ++i)
		{
			LeaderboardEntryUI entry;
			entry.carId = data[i];
			entry.y = 20.0f + (float)i * 24.0f;
			entry.targetY = entry.y;
			entry.animSpeed = 8.0f;
			entry.rank = i + 1;
			entry.lastRank = -1;
			entry.flashTimer = 0.0f;
			leaderboardUI.push_back(entry);
		}
	}

	for (int i = 0; i < (int)data.size(); ++i)
	{
		int id = data[i];

		for (auto& e : leaderboardUI)
		{
			if (e.carId == id)
			{
				e.targetY = 50.0f + (float)i * 24.0f;
				e.lastRank = e.rank;
				e.rank = i + 1;

				if (e.lastRank != -1 && e.lastRank != e.rank)
					e.flashTimer = 0.5f;

				break;
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

	for (int i = 0; i < (int)leaderboardUI.size(); ++i)
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

void ModuleUI::DrawResultsScreen()
{
	int y = 80;

	DrawText("RACE RESULTS", SCREEN_WIDTH / 2 - 100, y, 30, BLACK);
	y += 50;

	DrawText("FINAL POSITIONS", 80, y, 22, BLACK);
	y += 30;

	for (int i = 0; i < (int)App->scene_intro->results.finalLeaderboard.size(); ++i)
	{
		DrawText(TextFormat("%d. Car %d", i + 1, App->scene_intro->results.finalLeaderboard[i]),
			80, y, 18, BLACK);
		y += 22;
	}

	if (Button(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT - 120, 240, 60, "VOLVER AL MENU"))
		App->state->ChangeState(GameState::MENU_MAIN);
}

void ModuleUI::DrawRaceTimer()
{
	double time = App->scene_intro->GetRaceTime();

	int minutes = (int)(time / 60.0);
	int seconds = (int)time % 60;
	int millis = (int)((time - (int)time) * 1000);

	DrawText(TextFormat("%02d:%02d.%03d", minutes, seconds, millis),
		SCREEN_WIDTH / 2 - 80, 20, 30, BLACK);
}

void ModuleUI::UpdateRaceUI()
{
}
