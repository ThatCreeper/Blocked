#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>
#include <format>

static std::mt19937_64 rng;

inline float LerpPixelRound(float from, float to, float x, float max, int pixelSize = 16) {
	return LerpDistRound(from, to, x, max, 0.5f / pixelSize);
}


float randf() {
	return (float)GetRandomValue(0, RAND_MAX - 1) / (float)RAND_MAX;
}

struct Textures {
	void Load() {
	}

	void Unload() {
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
	rng = std::mt19937_64(std::random_device()());

	PlaySound(SND_START);

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();
	

	return restart;
}