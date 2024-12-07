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

	SND_COUNT
};

void LoadSounds();
Sound GetSound(SoundID id);
void PlaySound(SoundID id);
void StopSound(SoundID id);