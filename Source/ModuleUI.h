#pragma once
#include "Module.h"
#include "Globals.h"


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
private:
	
};