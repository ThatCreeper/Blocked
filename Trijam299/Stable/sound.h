#pragma once

#define RAYSNDS \
	S( SND_MENU, "menu.wav" ) \
	S( SND_START, "start.wav" ) \
	S( SND_DIE, "die.wav" ) \
	S( SND_GAME_OVER, "gameover.wav" ) \
	S( SND_REPAIR, "repair.wav" ) \
	S( SND_COMBO, "combo.wav" ) \
	S( SND_PROGRESS, "progress.wav" ) \
	S( SND_DETECTION, "detection.wav" ) \
	S( SND_EXPLOSION, "explosion.wav" ) \
	S( SND_FIRE, "fire.wav" ) \
	S( SND_MUSIC, "flua.ogg" ) \
	S( SND_WIN, "win.wav" ) \
	S( SND_HAT, "hat.ogg" ) \
	S( SND_SNARE, "snare.ogg" ) \
	S( SND_BASS, "bass.ogg" ) \
	S( SND_WAHWAHHITLER, "wahwahhitler.wav" ) \
	S( SND_SNIFF, "sniff.wav" ) \
	\
	M( SND_WOOFARF, "woofarf.ogg" ) \

enum SoundID {
#define S(a, b) a,
#define M(a, b) a,
	RAYSNDS
#undef S
#undef M

	SND_COUNT
};

void LoadSounds();
Sound GetSound(SoundID id);
void PlaySound(SoundID id);
void StopSound(SoundID id);
void SetMusic( SoundID id );
