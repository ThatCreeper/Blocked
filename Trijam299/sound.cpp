#include "global.h"

static SoundID snd_dont_repeat[] = {
	SND_MENU1,
};

Sound sndf_menu;
Sound sndf_menu1;
Sound sndf_start;
Sound sndf_underworld;
Sound sndf_web1;
Sound sndf_web2;
Sound sndf_web3;
Sound sndf_web4;
Sound sndf_ptr1;
Sound sndf_ptr2;
Sound sndf_ptr3;
Sound sndf_ptr4;
Sound sndf_archer;

void LoadSounds() {
	sndf_menu = LoadSound("snd/menu.wav");
	sndf_menu1 = LoadSoundAlias(sndf_menu);
	sndf_start = LoadSound("snd/start.wav");
	sndf_underworld = LoadSound("snd/underworld.ogg");
	sndf_web1 = LoadSound("snd/web1.ogg");
	sndf_web2 = LoadSound("snd/web2.ogg");
	sndf_web3 = LoadSound("snd/web3.ogg");
	sndf_web4 = LoadSound("snd/web4.ogg");
	sndf_ptr1 = LoadSound("snd/ptr1.ogg");
	sndf_ptr2 = LoadSound("snd/ptr2.ogg");
	sndf_ptr3 = LoadSound("snd/ptr3.ogg");
	sndf_ptr4 = LoadSound("snd/ptr4.ogg");
	sndf_archer = LoadSound("snd/archer.ogg");
}

Sound GetSound(SoundID id) {
	switch (id) {
	case SND_MENU:
		return(sndf_menu);
		break;
	case SND_MENU1:
		return(sndf_menu1);
		break;
	case SND_START:
		return(sndf_start);
		break;
	case SND_UNDERWORLD:
		return(sndf_underworld);
		break;
	case SND_WEB1:
		return(sndf_web1);
		break;
	case SND_WEB2:
		return(sndf_web2);
		break;
	case SND_WEB3:
		return(sndf_web3);
		break;
	case SND_WEB4:
		return(sndf_web4);
		break;
	case SND_PTR1:
		return(sndf_ptr1);
		break;
	case SND_PTR2:
		return(sndf_ptr2);
		break;
	case SND_PTR3:
		return(sndf_ptr3);
		break;
	case SND_PTR4:
		return(sndf_ptr4);
		break;
	case SND_ARCHER:
		return(sndf_archer);
		break;
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