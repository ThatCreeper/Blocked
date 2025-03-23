#pragma once

enum SoundID {
	SND_MENU,
	SND_START,
	SND_DIE,
	SND_GAME_OVER,
	SND_REPAIR,
	SND_COMBO,
	SND_PROGRESS,
	SND_PROGRESS1,
	SND_DETECTION,
	SND_EXPLOSION,
	SND_FIRE,
	SND_MENU1,
	SND_MUSIC,
	SND_WIN,
	SND_HAT,
	SND_SNARE,
	SND_BASS,
	SND_C,
	SND_D,
	SND_E,
	SND_F,
	SND_G,
	SND_A,
	SND_B,
	SND_C2,

	SND_COUNT
};

void LoadSounds();
Sound GetSound(SoundID id);
void PlaySound(SoundID id);
void StopSound(SoundID id);

namespace FMOD {
	namespace Studio {
		class EventInstance;
	}
}
using SoundInstance = FMOD::Studio::EventInstance *;
void InitFMod();
void CloseFMod();
void UpdateAudio();
SoundInstance MakeSound(const char *id);
void StartSound(SoundInstance snd);
void SetSoundParameter(SoundInstance snd, const char *param, float val);
void StopSound(SoundInstance snd);
void FireSound(const char *id);