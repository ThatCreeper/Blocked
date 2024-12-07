#include "global.h"

static SoundID snd_dont_repeat[] = {
	SND_PROGRESS,
	SND_PROGRESS1,
	SND_DIE,
	SND_GAME_OVER,
	SND_MENU1,
	SND_MUSIC
};

Sound sndf_menu;
Sound sndf_start;
Sound sndf_die;
Sound sndf_gover;
Sound sndf_repair;
Sound sndf_combo;
Sound sndf_progress;
Sound sndf_progress1;
Sound sndf_detection;
Sound sndf_explosion;
Sound sndf_fire;
Sound sndf_menu1;
Sound sndf_music;
Sound sndf_win;

void LoadSounds() {
	sndf_menu = LoadSound("snd/menu.wav");
	sndf_start = LoadSound("snd/start.wav");
	sndf_die = LoadSound("snd/die.wav");
	sndf_gover = LoadSound("snd/gameover.wav");
	sndf_repair = LoadSound("snd/repair.wav");
	sndf_combo = LoadSound("snd/combo.wav");
	sndf_progress = LoadSound("snd/progress.wav");
	sndf_progress1 = LoadSoundAlias(sndf_progress);
	sndf_detection = LoadSound("snd/detection.wav");
	sndf_explosion = LoadSound("snd/explosion.wav");
	sndf_fire = LoadSound("snd/fire.wav");
	sndf_menu1 = LoadSoundAlias(sndf_menu);
	sndf_music = LoadSound("snd/music.wav");
	sndf_win = LoadSound("snd/win.wav");
}

Sound GetSound(SoundID id) {
	switch (id) {
	case SND_MENU:
		return(sndf_menu);
		break;
	case SND_START:
		return(sndf_start);
		break;
	case SND_DIE:
		return(sndf_die);
		break;
	case SND_GAME_OVER:
		return(sndf_gover);
		break;
	case SND_REPAIR:
		return(sndf_repair);
		break;
	case SND_COMBO:
		return(sndf_combo);
		break;
	case SND_PROGRESS:
		return(sndf_progress);
		break;
	case SND_PROGRESS1:
		return(sndf_progress1);
		break;
	case SND_DETECTION:
		return(sndf_detection);
	case SND_EXPLOSION:
		return sndf_explosion;
	case SND_FIRE:
		return sndf_fire;
	case SND_MENU1:
		return sndf_menu1;
	case SND_MUSIC:
		return sndf_music;
	case SND_WIN:
		return sndf_win;
	}
}

void PlaySound(SoundID id) {
	bool dontrepeat = false;

	for (int i = 0; i < sizeof(snd_dont_repeat) / sizeof(*snd_dont_repeat); i++) {
		if (id == snd_dont_repeat[i]) {
			dontrepeat = true;
			break;
		}
	}

	if (dontrepeat && IsSoundPlaying(GetSound(id)))
		return;
	PlaySound(GetSound(id));
}

void StopSound(SoundID id) {
	StopSound(GetSound(id));
}