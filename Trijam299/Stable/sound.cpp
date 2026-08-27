#include "global.h"
#include "sound.h"
#include <array>

std::array<Sound, SND_COUNT> loadedSounds;

void LoadSounds() {
#define S(a, b) loadedSounds[a] = LoadSound(b);
	RAYSNDS
#undef S
}

Sound GetSound(SoundID id) {
	return loadedSounds[id];
}

void PlaySound(SoundID id) {
	if (id == SND_WOOFARF && IsSoundPlaying(GetSound(id))) return;
	PlaySound(GetSound(id));
}

void StopSound(SoundID id) {
	StopSound(GetSound(id));
}
