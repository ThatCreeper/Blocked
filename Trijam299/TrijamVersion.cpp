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
} s;

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();

	FireSound("startgame");

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		BeginDrawing();
		ClearBackground(BLACK);

		DoFadeInAnimation(fadein);

		UpdateAudio();
		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();
	

	return restart;
}