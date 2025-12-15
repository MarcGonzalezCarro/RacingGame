#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleUI.h"
#include "ModuleRender.h"
#include "ModuleState.h"
#include <math.h>

ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    
}

// Destructor
ModuleUI::~ModuleUI()
{}


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
		// UI in-game (si quieres)
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

bool ModuleUI::Button(int x, int y, int w, int h, const char* text)
{
	Vector2 mouse = { (float)GetMouseX(), (float)GetMouseY() };
	Rectangle rect = { (float)x, (float)y, (float)w, (float)h };

	bool hover = CheckCollisionPointRec(mouse, rect);

	
	App->renderer->DrawUIButton(x, y, w, h, text, hover);

	if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		return true;

	return false;
}

void ModuleUI::UpdateMainMenu()
{
	int cx = SCREEN_WIDTH / 2 - 100;

	if (Button(cx, 200, 200, 50, "JUGAR"))
	{
		App->state->ChangeState(GameState::MENU_PLAY);
	}

	if (Button(cx, 270, 200, 50, "CONFIG"))
	{
		App->state->ChangeState(GameState::MENU_OPTIONS);
	}
}
void ModuleUI::UpdatePlayMenu()
{
	int cx = SCREEN_WIDTH / 2 - 100;

	if (Button(cx, 220, 200, 50, "JUGAR"))
	{
		App->state->ChangeState(GameState::RACE);
	}

	if (Button(cx, 290, 200, 50, "ATRAS"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}
void ModuleUI::UpdateOptionsMenu()
{
	int cx = SCREEN_WIDTH / 2 - 100;

	if (Button(cx, 260, 200, 50, "ATRAS"))
	{
		App->state->ChangeState(GameState::MENU_MAIN);
	}
}