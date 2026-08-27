#pragma once

#define RAYSNDS \
	S(SND_MENU, "snd/menu.wav") \
	S(SND_START, "snd/start.wav") \
	S(SND_DIE, "snd/die.wav") \
	S(SND_GAME_OVER, "snd/gameover.wav") \
	S(SND_REPAIR, "snd/repair.wav") \
	S(SND_COMBO, "snd/combo.wav") \
	S(SND_PROGRESS, "snd/progress.wav") \
	S(SND_DETECTION, "snd/detection.wav") \
	S(SND_EXPLOSION, "snd/explosion.wav") \
	S(SND_FIRE, "snd/fire.wav") \
	S(SND_MUSIC, "snd/flua.ogg") \
	S(SND_WIN, "snd/win.wav") \
	S(SND_HAT, "snd/hat.ogg") \
	S(SND_SNARE, "snd/snare.ogg") \
	S(SND_BASS, "snd/bass.ogg") \
	S(SND_WOOFARF, "snd/woofarf.ogg") \
	S(SND_WAHWAHHITLER, "snd/wahwahhitler.wav") \
	S(SND_SNIFF, "snd/sniff.wav")

enum SoundID {
#define S(a, b) a,
	RAYSNDS
#undef S

	SND_COUNT
};

void LoadSounds();
Sound GetSound(SoundID id);
void PlaySound(SoundID id);
void StopSound(SoundID id);
