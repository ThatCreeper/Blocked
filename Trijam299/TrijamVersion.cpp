#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"

import world;

json entities;
Image collisionI;

#define TEXTURES \
	T(frozen, "frozen.png") \
	T(map, "ldtk/map/simplified/Level_0/_composite.png")
struct Textures {
#define T(a, b) Texture2D a;
	TEXTURES

	void Load() {
#define T(a, b) a = LoadTexture(b);
		TEXTURES
	}

	void Unload() {
#define T(a, b) UnloadTexture(a);
		TEXTURES
	}
};
#undef T
#undef TEXTURES

struct State {
	Textures t;
	flux::Group g;
	SoundInstance mus;

	int px;
	float pfx;
	float pvx;
	int py;
	float pfy;
	float pvy;
	float timeSinceMove = 0;
} s;

void LoadEntities() {
	// TODO: memory leak
	collisionI = LoadImage("ldtk/map/simplified/Level_0/Collision-int.png");
	char *fileText = LoadFileText("ldtk/map/simplified/Level_0/data.json");
	entities = json::parse(fileText)["entities"];
	UnloadFileText(fileText);
}

bool CollidesWorldTile(int tx, int ty) {
	if (tx >= collisionI.width)
		return false;
	if (ty >= collisionI.height)
		return false;
	if (tx < 0)
		return false;
	if (ty < 0)
		return false;
	return GetImageColor(collisionI, tx, ty).b == 255;
}

bool CollidesWorld(int x, int y) {
	return CollidesWorldTile(x / 8, y / 8);
}

bool CollidesWorld(int x, int y, int w, int h) {
	int tx1 = x / 8;
	int ty1 = y / 8;
	int tx2 = (x + w - 1) / 8;
	int ty2 = (y + h - 1) / 8;
	for (int y = ty1; y <= ty2; y++) {
		for (int x = tx1; x <= tx2; x++) {
			if (CollidesWorldTile(x, y))
				return true;
		}
	}
	return false;
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	LoadEntities();
	for (const auto &p : entities["Player"]) {
		if (p["customFields"]["id"] != 0)
			continue;
		s.px = p["x"];
		s.py = p["y"];
		break;
	}
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

		bool playerOnGround = CollidesWorld(s.px, s.py + 8, 5, 1);
		s.timeSinceMove += GetFrameTime();
		float timeMod = 1.f / (1 + s.timeSinceMove * 5);

		s.pvx *= std::powf(0.7f, timeMod);
		if (IsKeyDown(KEY_A)) {
			s.pvx -= 0.5 * timeMod;
			s.timeSinceMove = 0;
		}
		if (IsKeyDown(KEY_D)) {
			s.pvx += 0.5 * timeMod;
			s.timeSinceMove = 0;
		}
		if (playerOnGround && s.pvy > 0) {
			s.pvy = 0;
		}
		if (playerOnGround && IsKeyPressed(KEY_W)) {
			s.pvy = -3;
			s.timeSinceMove = 0;
		}
		s.pvy += timeMod * (s.pvy > 0 ? 0.1 : 0.3);
		s.pfy += s.pvy * timeMod;
		while (s.pfy > 0) {
			if (CollidesWorld(s.px, s.py + 1, 5, 8)) {
				s.pfy = 0;
				break;
			}
			s.py++;
			s.pfy--;
		}
		while (s.pfy < 0) {
			if (CollidesWorld(s.px, s.py - 1, 5, 8)) {
				s.pfy = 0;
				break;
			}
			s.py--;
			s.pfy++;
		}
		s.pfx += s.pvx * timeMod;
		while (s.pfx > 0) {
			if (CollidesWorld(s.px + 1, s.py, 5, 8)) {
				s.pfx = 0;
				break;
			}
			s.px++;
			s.pfx--;
		}
		while (s.pfx < 0) {
			if (CollidesWorld(s.px - 1, s.py, 5, 8)) {
				s.pfx = 0;
				break;
			}
			s.px--;
			s.pfx++;
		}

		BeginDrawing();
		ClearBackground(BLACK);
		rlImGuiBegin();

		ImGui::Begin("Test Window");

		ImGui::Text("Hello!");

		ImGui::End();

		DrawTexture(s.t.map, 0, 0, WHITE);
		DrawRectangle(s.px, s.py, 5, 8, RED);

		DoFadeInAnimation(fadein);

		UpdateAudio();
		rlImGuiEnd();
		EndDrawing();
	}

END:

	StopSound(s.mus);
	SaveGlobState();
	s.t.Unload();
	

	return restart;
}