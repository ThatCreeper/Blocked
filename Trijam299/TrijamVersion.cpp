#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>

static std::mt19937_64 rng;

static struct Particle {
	float r;
	int x;
	int sy;
	float y;
	float T;
};

static void RandomizeParticle(Particle &p) {
	p.r = (float)GetRandomValue(0, 360) / 180.f * PI;
	p.y = p.sy;
	p.T = GetRandomValue(0, 10) / 6.f;
}

static void AddParticles(int sx, int sy) {
	for (int i = 0; i < 12; i++) {
		Particle p;
		p.x = sx;
		p.sy = sy;
		RandomizeParticle(p);
		// particles.push_back(p);
	}
}


inline float LerpPixelRound(float from, float to, float x, float max, int pixelSize = 16) {
	return LerpDistRound(from, to, x, max, 0.5f / pixelSize);
}

struct Textures {

	void Load() {
	}

	void Unload() {
	}
};

struct Player {
	float x;
	float y;
};

struct State {
	Textures t;
	flux::Group g;
} s;

static bool GameOver() {
	StopSound(SND_MUSIC);
	const char *t = TextFormat("You made $d!");
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText(t, (800 - MeasureText(t, 60)) / 2, 100, 60, WHITE);
		//DrawKeybindBar("", "");
		EndDrawing();
	}
	return false;
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	rng = std::mt19937_64(0x007ABCD);

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		//PlaySound(SND_MUSIC);

		BeginDrawing();

		ClearBackground(BLACK);

		EndMode2D();

		DrawKeybindBar("[Up] [Down] [Left] [Right]", "");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();

	//StopSound(SND_MUSIC);

	return restart;
}