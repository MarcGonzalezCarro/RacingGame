#pragma once
#include "Module.h"
#include "Globals.h"

enum class GameState
{
	NONE,
	MENU_MAIN,
	MENU_PLAY,
	MENU_OPTIONS,
	RACE,
	RESULTS
};

class ModuleState : public Module
{
public:
	ModuleState(Application* app, bool start_enabled = true);
	~ModuleState();

	bool Init();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void ChangeState(GameState newState);
	GameState GetState() const { return currentState; }
	int mapId = 1;
private:
	GameState currentState = GameState::MENU_MAIN;
	GameState nextState = GameState::MENU_MAIN;
	bool menuMusicPlaying = false;
	bool endSongPlayed = false;

	void ProcessStateChange();

	// ---- State callbacks ----
	void OnEnterMenu();
	void OnExitMenu();

	void OnEnterRace();
	void OnExitRace();

	void OnEnterResults();
	void OnExitResults();



};
