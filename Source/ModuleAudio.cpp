#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"

#include "raylib.h"

static float Clamp01(float v)
{
	if (v < 0.0f) return 0.0f;
	if (v > 1.0f) return 1.0f;
	return v;
}

ModuleAudio::ModuleAudio(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	fx_count = 0;
	music = Music{ 0 };
	sfxVolume = 1.0f;
	musicVolume = 1.0f;
	musicEnabled = true;

	// Initialize FX array to a known state
	for (unsigned int i = 0; i < MAX_SOUNDS; ++i)
	{
		fx[i] = Sound{ 0 };
	}
}

ModuleAudio::~ModuleAudio()
{
}

bool ModuleAudio::Init()
{
	LOG("Loading Audio Mixer");
	InitAudioDevice();
	return true;
}

update_status ModuleAudio::Update()
{
	if (IsMusicReady(music))
	{
		UpdateMusicStream(music);
	}
	return UPDATE_CONTINUE;
}

bool ModuleAudio::CleanUp()
{
	LOG("Freeing sound FX, closing Audio subsystem");

	for (unsigned int i = 0; i < fx_count; ++i)
	{
		UnloadSound(fx[i]);
	}

	StopMusic();

	CloseAudioDevice();
	return true;
}

bool ModuleAudio::PlayMusic(const char* path, float fade_time, bool loop)
{
	(void)fade_time;

	if (!IsEnabled() || !musicEnabled)
		return false;

	StopMusic();

	music = LoadMusicStream(path);
	music.looping = loop;

	if (!IsMusicReady(music))
	{
		LOG("Cannot load music: %s", path);
		music = Music{ 0 };
		return false;
	}

	::SetMusicVolume(music, musicVolume);
	PlayMusicStream(music);

	LOG("Successfully playing %s", path);
	return true;
}

void ModuleAudio::StopMusic()
{
	if (IsMusicReady(music))
	{
		StopMusicStream(music);
		UnloadMusicStream(music);
		music = Music{ 0 };
	}
}

unsigned int ModuleAudio::LoadFx(const char* path)
{
	if (!IsEnabled())
		return 0;

	if (fx_count >= MAX_SOUNDS)
	{
		LOG("Cannot load sound, MAX_SOUNDS reached: %s", path);
		return 0;
	}

	Sound sound = LoadSound(path);

	if (sound.stream.buffer == NULL)
	{
		LOG("Cannot load sound: %s", path);
		return 0;
	}

	fx[fx_count] = sound;
	SetSoundVolume(fx[fx_count], sfxVolume);

	return fx_count++;
}

bool ModuleAudio::PlayFx(unsigned int id, int repeat)
{
	(void)repeat;

	if (!IsEnabled() || id >= fx_count)
		return false;

	// Keeps original behavior: does not restart if already playing
	if (!IsSoundPlaying(fx[id]))
	{
		SetSoundVolume(fx[id], sfxVolume);
		PlaySound(fx[id]);
	}

	return true;
}

void ModuleAudio::StopFx(unsigned int id)
{
	if (id < fx_count)
		StopSound(fx[id]);
}

void ModuleAudio::SetSfxVolume(float volume)
{
	sfxVolume = Clamp01(volume);
	ApplySfxVolume();
}

void ModuleAudio::SetMusicVolume(float volume)
{
	musicVolume = Clamp01(volume);

	if (IsMusicReady(music))
	{
		::SetMusicVolume(music, musicVolume);
	}
}

float ModuleAudio::GetSfxVolume() const
{
	return sfxVolume;
}

float ModuleAudio::GetMusicVolume() const
{
	return musicVolume;
}

void ModuleAudio::SetMusicEnabled(bool enabled)
{
	musicEnabled = enabled;

	if (!musicEnabled)
	{
		StopMusic();
	}
}

bool ModuleAudio::IsMusicEnabled() const
{
	return musicEnabled;
}

void ModuleAudio::ApplySfxVolume()
{
	for (unsigned int i = 0; i < fx_count; ++i)
	{
		SetSoundVolume(fx[i], sfxVolume);
	}
}
