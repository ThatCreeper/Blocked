#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"

import world;
import stateinator;

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
		break;
	}
	s.mus = MakeSound("bgmus");
	SetSoundParameter(s.mus, "intensity", 0);
	StartSound(s.mus);

	FireSound("startgame");

	RenderTexture2D render = LoadRenderTexture(SCRWID, SCRHEI);
	Shader shader = LoadShader(NULL, "blur.fs");

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		BeginTextureMode(render);

		ClearBackground(RED);

		DrawRectangle(50, 50, 80, 80, GREEN);

		EndTextureMode();

		BeginDrawing();
		rlImGuiBegin();
		
		ClearBackground(BLACK);

		BeginShaderMode(shader);

		DrawTexturePro(render.texture, { 0, 0, SCRWID, -SCRHEI }, { 0, 0, SCRWID, SCRHEI }, { 0, 0 }, 0, WHITE);

		EndShaderMode();

		ImGui::Begin("Test Window");

		ImGui::Text("Hello!");
		ImGui::Text("FPS: %f", 1.0 / GetFrameTime());

		if (ImGui::Button("Reload Shader")) {
			UnloadShader(shader);
			shader = LoadShader(nullptr, "blur.fs");
		}

		ImGui::End();


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