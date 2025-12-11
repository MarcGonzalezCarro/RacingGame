#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include <math.h>

ModuleRender::ModuleRender(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    background = RAYWHITE;
}

// Destructor
ModuleRender::~ModuleRender()
{}

// Called before render is available
bool ModuleRender::Init()
{
	LOG("Creating Renderer context");
	bool ret = true;

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRender::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// Update: debug camera
update_status ModuleRender::Update()
{
    ClearBackground(background);

    // NOTE: This function setups render batching system for
    // maximum performance, all consecutive Draw() calls are
    // not processed until EndDrawing() is called
    BeginDrawing();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRender::PostUpdate()
{
    // Draw everything in our batch!
    DrawFPS(10, 10);

    EndDrawing();

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRender::CleanUp()
{
	return true;
}

void ModuleRender::SetBackgroundColor(Color color)
{
	background = color;
}

// Draw to screen
bool ModuleRender::Draw(Texture2D texture, int x, int y, const Rectangle* section, double angle, int pivot_x, int pivot_y) const
{
	bool ret = true;

    float scale = 1.0f;

    // Rectángulo de origen
    Rectangle src = { 0.f, 0.f, (float)texture.width, (float)texture.height };
    if (section != nullptr)
        src = *section;

    // Rectángulo de destino: posición del sprite (coincide con el cuerpo)
    Rectangle dest = {
        (float)(x) + camera.x, 
        (float)(y) + camera.y,
        src.width * scale,
        src.height * scale
    };

    // Pivot: punto de rotación dentro del sprite
    Vector2 origin = { (float)pivot_x, (float)pivot_y };

    DrawTexturePro(texture, src, dest, origin, (float)angle, WHITE);

	return ret;
}

bool ModuleRender::DrawCar(Texture2D texture, int x, int y, const Rectangle* section, double angle, int pivot_x, int pivot_y) const
{
    bool ret = true;

    float scale = 1.0f;

    // Rectángulo de origen
    Rectangle src = { 0.f, 0.f, (float)texture.width, (float)texture.height };
    if (section != nullptr)
        src = *section;

    // Rectángulo de destino: posición del sprite (coincide con el cuerpo)
    Rectangle dest = {
        (float)(x),
        (float)(y),
        src.width * scale,
        src.height * scale
    };

    // Pivot: punto de rotación dentro del sprite
    Vector2 origin = { (float)pivot_x, (float)pivot_y };

    DrawTexturePro(texture, src, dest, origin, (float)angle, WHITE);

    return ret;
}

bool ModuleRender::DrawText(const char * text, int x, int y, Font font, int spacing, Color tint) const
{
    bool ret = true;

    Vector2 position = { (float)x, (float)y };

    DrawTextEx(font, text, position, (float)font.baseSize, (float)spacing, tint);

    return ret;
}
