#pragma once

enum SoundID {
	SND_MENU,
	SND_MENU1,
	SND_START,
	SND_UNDERWORLD,
	SND_WEB1,
	SND_WEB2,
	SND_WEB3,
	SND_WEB4,
	SND_PTR1,
	SND_PTR2,
	SND_PTR3,
	SND_PTR4,
	SND_ARCHER,

	SND_COUNT
};

void LoadSounds();
Sound GetSound(SoundID id);
void PlaySound(SoundID id);
void StopSound(SoundID id);