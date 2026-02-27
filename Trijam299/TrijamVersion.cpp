#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"
#include "entity.h"

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
#undef T

	void Load() {
#define T(a, b) a = LoadTexture(b);
		TEXTURES
#undef T
	}

	void Unload() {
#define T(a, b) UnloadTexture(a);
		TEXTURES
#undef T
	}

	void Gui() {
		ImGui::Begin("Textures");
		ImGui::BeginGroup();

#define T(a, b) if (ImGui::Button(#a)) { UnloadTexture(a); a = LoadTexture(b); }
		TEXTURES
#undef T

		ImGui::EndGroup();
		ImGui::End();
	}
};
#undef TEXTURES

#define SHADERS \
	T(blur, nullptr, "blur.fs")
struct Shaders {
#define T(a, b, c) Shader a;
	SHADERS
#undef T

	void Load() {
#define T(a, b, c) a = LoadShader(b, c);
		SHADERS
#undef T
	}

	void Unload() {
#define T(a, b, c) UnloadShader(a);
		SHADERS
#undef T
	}

	void Gui() {
		ImGui::Begin("Shaders");
		ImGui::BeginGroup();

#define T(a, b, c) if (ImGui::Button(#a)) { Shader s = LoadShader(b, c); if (s.id) { UnloadShader(a); a = s; } }
		SHADERS
#undef T

		ImGui::EndGroup();
		ImGui::End();
	}
};
#undef SHADERS

struct State {
	Textures t;
	Shaders s;
	flux::Group g;
	SoundInstance mus;
	world w;
} s;

struct testent : entity {
	E_SLS
		E_SL(SIGNAL_GUI);
	E_SLE

	E_SIGNAL(SIGNAL_GUI) {
		if (ImGui::CollapsingHeader("testent")) {
			if (ImGui::Button("kill")) w->remove(this);
		}
	}
};

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
	s.s.Load();
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

	s.w.add(new testent);

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		s.w.update();

		BeginTextureMode(render);

		ClearBackground(BLACK);

		DrawRectangle(50, 50, 80, 80, WHITE);

		s.w.render();

		EndTextureMode();

		BeginDrawing();
		rlImGuiBegin();
		
		ClearBackground(BLACK);

		BeginShaderMode(s.s.blur);

		DrawTexturePro(render.texture, { 0, 0, SCRWID, -SCRHEI }, { 0, 0, SCRWID, SCRHEI }, { 0, 0 }, 0, WHITE);

		EndShaderMode();

		s.t.Gui();
		s.s.Gui();
		ImGui::Begin("Entities");
		s.w.broadcast(SIGNAL_GUI, nullptr);
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
	s.s.Unload();

	return restart;
}