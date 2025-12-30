#pragma once

#include "Module.h"
#include "Globals.h"
#include <vector>

// UI data used to animate leaderboard entries
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

	// Generic UI button. Returns true when clicked
	bool Button(int x, int y, int w, int h, const char* text);

	// Menu states
	void UpdateMainMenu();
	void UpdatePlayMenu();
	void UpdateOptionsMenu();

	// Race UI
	void UpdateRaceUI();
	void SyncLeaderboard();
	void UpdateLeaderboardAnimation();
	void DrawLeaderboard();

	// Results and timer
	void DrawResultsScreen();
	void DrawRaceTimer();

private:
	// Leaderboard visual data
	std::vector<LeaderboardEntryUI> leaderboardUI;

	// UI sound effects
	unsigned int pressFx = 0;
};
