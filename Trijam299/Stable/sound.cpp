#include "global.h"
#include "sound.h"
#include <array>

static std::array<Sound, SND_COUNT> loadedSounds;
static bool loopedSounds[] = {
#define S(a, b) false,
#define M(a, b) true,
	RAYSNDS
#undef S
#undef M
};

void LoadSounds() {
#define S(a, b) loadedSounds[a] = LoadSound("snd/" b);
#define M(a, b) S(a, b)
	RAYSNDS
#undef S
#undef M
}

Sound GetSound(SoundID id) {
	return loadedSounds[id];
}

void PlaySound(SoundID id) {
	if (loopedSounds[id] && IsSoundPlaying(GetSound(id)) ) return;
	PlaySound(GetSound(id));
}

void StopSound(SoundID id) {
	StopSound(GetSound(id));
}

void SetMusic( SoundID id )
{
	for ( int i = 0; i < SND_COUNT; i++ )
	{
		if ( i == id ) continue;
		if ( !loopedSounds[i] ) continue;

		if ( IsSoundPlaying( loadedSounds[i] ) )
		{
			StopSound( loadedSounds[i] );
		}
	}
	PlaySound( loadedSounds[id] );
}
