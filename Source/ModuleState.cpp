#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleState.h"
#include "ModuleGame.h"
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
	App->scene_intro->CreateMockUpCar();
	App->scene_intro->onMenu = true;
	// Mostrar UI menú
}

void ModuleState::OnExitMenu()
{
	LOG("EXIT MENU");
	App->scene_intro->DestroyMockUpCar();
}

void ModuleState::OnEnterRace()
{
	LOG("STATE ? RACE");
	App->scene_intro->DestroyMockUpCar();
	App->scene_intro->CreateRace(800,1750,50,100,1.0f,3);
	
}

void ModuleState::OnExitRace()
{
	LOG("EXIT RACE");
	App->scene_intro->DeleteRace();
	App->scene_intro->onRace = false;
}

void ModuleState::OnEnterResults()
{
	LOG("STATE ? RESULTS");
	printf("Race Finish\n");
	ChangeState(GameState::MENU_MAIN);
}

void ModuleState::OnExitResults()
{
	printf("EXIT RESULTS\n");
}