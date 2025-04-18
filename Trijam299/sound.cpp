#include "global.h"
#ifdef _USE_FMOD_
#include <fmod_studio.hpp>
#include <unordered_map>
#include <string>
#endif

void LoadSounds() {
	// noop
}

Sound GetSound(SoundID id) {
	// noop
	return { 0 };
}

void PlaySound(SoundID id) {
	// noop
}

void StopSound(SoundID id) {
	// noop
}

#ifdef _USE_FMOD_
FMOD::Studio::System *sys;
FMOD::Studio::Bank *bank;
FMOD::Studio::Bank *strbank;

void FASSERT(FMOD_RESULT res) {
	assert(res == FMOD_OK);
}

void InitFMod() {
	FMOD::Studio::System::create(&sys);
	FMOD_STUDIO_INITFLAGS flags;
#ifdef _DEBUG
	flags = FMOD_STUDIO_INIT_LIVEUPDATE;
#else
	flags = FMOD_STUDIO_INIT_NORMAL;
#endif
	FASSERT(sys->initialize(512, flags, FMOD_INIT_NORMAL, nullptr));
	FASSERT(sys->loadBankFile("Desktop/Master.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
	FASSERT(sys->loadBankFile("Desktop/Master.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &strbank));

#ifdef DEBUG_LISTEVENTS
	FMOD::Studio::EventDescription *descs[256];
	int count = 0;
	FASSERT(bank->getStringCount(&count));
	printf("stringcount = %d\n", count);
	FASSERT(sys->getBankCount(&count));
	printf("bankcount = %d\n", count);
	FASSERT(bank->getEventList(descs, 256, &count));
	for (int i = 0; i < count; i++) {
		char name[256];
		int ncount = 0;
		FASSERT(descs[i]->getPath(name, 256, &ncount));
		printf("snd: %.*s\n", ncount, name);
	}
#endif
}

void CloseFMod() {
	FASSERT(bank->unload());
	FASSERT(sys->release());
	sys = nullptr;
}

void UpdateAudio() {
	FASSERT(sys->update());
}

SoundInstance MakeSoundByPath(const char *id) {
	FMOD::Studio::EventInstance *ei;
	FMOD::Studio::EventDescription *ed;
	FMOD_RESULT res = sys->getEvent(id, &ed);
	if (res == FMOD_ERR_EVENT_NOTFOUND) {
		printf("SND: No event '%s'!\n", id);
		return nullptr;
	}
	FASSERT(res);
	FASSERT(ed->createInstance(&ei));
	return ei;
}

SoundInstance MakeSound(const char *id) {
	char path[256] = "event:/";
	strcpy_s(path + 7, 256 - 7, id);
	return MakeSoundByPath(path);
}

void StartSound(SoundInstance ei) {
	if (ei == nullptr)
		return;
	FASSERT(ei->start());
	FASSERT(ei->release());
}

void SetSoundParameter(SoundInstance ei, const char *param, float val) {
	if (ei == nullptr)
		return;
	FASSERT(ei->setParameterByName(param, val));
}

void StopSound(SoundInstance ei) {
	if (ei == nullptr)
		return;
	FASSERT(ei->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
}

void FireSound(const char *id) {
	StartSound(MakeSound(id));
}
#else
void InitFMod() {}
void CloseFMod() {}
void UpdateAudio() {}
SoundInstance MakeSound(const char *id) { return nullptr; }
void StartSound(SoundInstance snd) {}
void SetSoundParameter(SoundInstance snd, const char *param, float val) {}
void StopSound(SoundInstance snd) {}
void FireSound(const char *id) {}
#endif