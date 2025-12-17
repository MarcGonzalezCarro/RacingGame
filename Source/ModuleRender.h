#pragma once
#include "Module.h"
#include "Globals.h"

#include <limits.h>

class ModuleRender : public Module
{
public:
	ModuleRender(Application* app, bool start_enabled = true);
	~ModuleRender();

	bool Init();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

    void SetBackgroundColor(Color color);
	bool Draw(Texture2D texture, int x, int y, const Rectangle* section = NULL, double angle = 0, int pivot_x = 0, int pivot_y = 0, float scale = 0) const;
    bool DrawText(const char* text, int x, int y, Font font, int spacing, Color tint) const;

	void DrawUIButton(int x, int y, int w, int h, const char* text, bool hover);

public:

	Color background;
    Rectangle camera;
	Font fuente;
};