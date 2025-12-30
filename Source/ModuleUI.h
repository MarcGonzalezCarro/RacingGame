#pragma once

#include "Module.h"
#include "Globals.h"
#include <vector>

struct LeaderboardEntryUI
{
	int carId = -1;
	float y = 0.0f;
	float targetY = 0.0f;
	float animSpeed = 0.0f;
	int rank = -1;
	int lastRank = -1;
	float flashTimer = 0.0f;
};

class ModuleUI : public Module
{
public:
	ModuleUI(Application* app, bool start_enabled = true);
	~ModuleUI();

	bool Init();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	bool Button(int x, int y, int w, int h, const char* text);

	void UpdateMainMenu();
	void UpdatePlayMenu();
	void UpdateOptionsMenu();

	void UpdateRaceUI();
	void SyncLeaderboard();
	void UpdateLeaderboardAnimation();
	void DrawLeaderboard();

	void DrawResultsScreen();
	void DrawRaceTimer();

private:
	std::vector<LeaderboardEntryUI> leaderboardUI;

	unsigned int pressFx = 0;

	bool draggingSfx = false;
	bool draggingMusic = false;
};
