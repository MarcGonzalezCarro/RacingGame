#pragma once

#include "Module.h"
#include "raylib.h"

#define MAX_SOUNDS 16
#define DEFAULT_MUSIC_FADE_TIME 2.0f

class ModuleAudio : public Module
{
public:
	ModuleAudio(Application* app, bool start_enabled = true);
	~ModuleAudio();

	bool Init();
	update_status Update();
	bool CleanUp();

	// Music
	bool PlayMusic(const char* path, float fade_time = DEFAULT_MUSIC_FADE_TIME, bool loop = true);
	void StopMusic();

	// FX
	unsigned int LoadFx(const char* path);
	bool PlayFx(unsigned int fx, int repeat = 0);
	void StopFx(unsigned int id);

	// Audio settings
	void SetSfxVolume(float volume);    // 0.0f - 1.0f
	void SetMusicVolume(float volume);  // 0.0f - 1.0f
	float GetSfxVolume() const;
	float GetMusicVolume() const;

	void SetMusicEnabled(bool enabled);
	bool IsMusicEnabled() const;

private:
	void ApplySfxVolume();

	Music music;
	Sound fx[MAX_SOUNDS];
	unsigned int fx_count;

	float sfxVolume;
	float musicVolume;
	bool musicEnabled;
};
