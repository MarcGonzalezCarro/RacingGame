#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleState.h"
#include <math.h>

ModuleState::ModuleState(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    
}

// Destructor
ModuleState::~ModuleState()
{}

// Called before render is available
bool ModuleState::Init()
{
	LOG("Creating Renderer context");
	bool ret = true;

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleState::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// Update: debug camera
update_status ModuleState::Update()
{
	ProcessStateChange();

	switch (currentState)
	{
	case GameState::MENU_MAIN:
		//UpdateMenu();
		break;

	case GameState::RACE:
		//UpdateRace();
		break;

	case GameState::RESULTS:
		//UpdateResults();
		break;

	default:
		break;
	}

	return UPDATE_CONTINUE;

}

// PostUpdate present buffer to screen
update_status ModuleState::PostUpdate()
{
 

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleState::CleanUp()
{
	return true;
}

void ModuleState::ChangeState(GameState newState)
{
	if (newState == currentState)
		return;

	nextState = newState;
}

void ModuleState::ProcessStateChange()
{
	if (nextState == GameState::NONE)
		return;

	// Exit current
	switch (currentState)
	{
	case GameState::MENU_MAIN:    OnExitMenu(); break;
	case GameState::RACE:    OnExitRace(); break;
	case GameState::RESULTS: OnExitResults(); break;
	default: break;
	}

	currentState = nextState;
	nextState = GameState::NONE;

	// Enter new
	switch (currentState)
	{
	case GameState::MENU_MAIN:    OnEnterMenu(); break;
	case GameState::RACE:    OnEnterRace(); break;
	case GameState::RESULTS: OnEnterResults(); break;
	default: break;
	}
}

void ModuleState::OnEnterMenu()
{
	LOG("STATE ? MENU");

	// Mostrar UI menú
}

void ModuleState::OnExitMenu()
{
	LOG("EXIT MENU");
}

void ModuleState::OnEnterRace()
{
	LOG("STATE ? RACE");

	
}

void ModuleState::OnExitRace()
{
	LOG("EXIT RACE");

}

void ModuleState::OnEnterResults()
{
	LOG("STATE ? RESULTS");

	
}

void ModuleState::OnExitResults()
{
	LOG("EXIT RESULTS");
}