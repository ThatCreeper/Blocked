#include "raylib.h"

#include "external/glad.h"

#include "global.h"

struct Textures {
	Texture2D frozen;

	void Load() {
		frozen = LoadTexture("frozen.png");
	}

	void Unload() {
		UnloadTexture(frozen);
	}
};

struct State {
	Textures t;
	flux::Group g;
	SoundInstance mus;
} s;

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	s.mus = MakeSound("bgmus");
	SetSoundParameter(s.mus, "intensity", 0);
	StartSound(s.mus);

	FireSound("startgame");

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		if (IsKeyPressed(KEY_ONE)) {
			SetSoundParameter(s.mus, "intensity", 0);
		}
		else if (IsKeyPressed(KEY_TWO)) {
			SetSoundParameter(s.mus, "intensity", 1);
		}
		else if(IsKeyPressed(KEY_THREE)) {
			SetSoundParameter(s.mus, "intensity", 2);
		}
		else if(IsKeyPressed(KEY_FOUR)) {
			SetSoundParameter(s.mus, "intensity", 3);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		DoFadeInAnimation(fadein);

		UpdateAudio();
		EndDrawing();
	}

END:

	StopSound(s.mus);
	SaveGlobState();
	s.t.Unload();
	

	return restart;
}