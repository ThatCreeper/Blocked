#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"
#include "entity.h"

import world;
import stateinator;

#define TEXTURES \
	T(frozen, "frozen.png") \
	T(baselut, "baselut.png")
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
#define UNIFORMS \
	U(blur, lut, "lut")
struct Shaders {
#define T(a, b, c) Shader a;
	SHADERS;
#undef T
#define U(a, b, c) int uniform_##a##_##b;
	UNIFORMS;
#undef U

	void Load() {
#define T(a, b, c) a = LoadShader(b, c);
		SHADERS;
#undef T
		LoadUniforms();
	}

	void LoadUniforms() {
#define U(a, b, c) uniform_##a##_##b = GetShaderLocation(a, c);
		UNIFORMS;
#undef U
	}

	void Unload() {
#define T(a, b, c) UnloadShader(a);
		SHADERS;
#undef T
	}

	void Gui() {
		ImGui::Begin("Shaders");
		ImGui::BeginGroup();

#define T(a, b, c) if (ImGui::Button(#a)) { Shader s = LoadShader(b, c); if (s.id) { UnloadShader(a); a = s; LoadUniforms(); } }
		SHADERS;
#undef T

		ImGui::EndGroup();
		ImGui::End();
	}
};
#undef SHADERS
#undef UNIFORMS

struct State {
	Textures t;
	Shaders s;
	flux::Group g;
	world w;
} s;

struct positional : entity {
	int x = 0;
	int y = 0;

	E_SLS
		E_SL(SIGNAL_GUI);
	E_SLE;

	E_SIGNAL(SIGNAL_GUI) {
		if (guiHeader("positional")) {
			if (ImGui::Button("kill")) w->remove(this);
			ImGui::DragInt("x", &x);
			ImGui::DragInt("y", &y);

			ImGui::TreePop();
		}
	}
};

struct rectRend : entityRenderer {
	Color color_;
	int wid_;
	int hei_;
	int offX_;
	int offY_;

	rectRend(entity *_e, Color color, int wid, int hei, int offX, int offY) : entityRenderer(_e) {
		color_ = color;
		wid_ = wid;
		hei_ = hei;
		offX_ = offX;
		offY_ = offY;
	}

	void render(entity *_e) override {
		ER_E(positional);
		DrawRectangle(e->x + offX_, e->y + offY_, wid_, hei_, color_);
	}
};

struct testent : positional {
	E_REND(new rectRend(this, BLUE, 32, 32, 0, 0));

	E_SLS
		E_SL(SIGNAL_GUI);
	E_SLE;

	E_SIGNAL(SIGNAL_GUI) {
		if (guiHeader("testent")) {
			E_SIGPAR(positional, SIGNAL_GUI);

			ImGui::TreePop();
		}
	}
};

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	s.s.Load();

	PlaySound(SND_START);

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
		SetShaderValueTexture(s.s.blur, s.s.uniform_blur_lut, s.t.baselut);

		DrawTexturePro(render.texture, { 0, 0, SCRWID, -SCRHEI }, { 0, 0, SCRWID, SCRHEI }, { 0, 0 }, 0, WHITE);

		EndShaderMode();

		s.t.Gui();
		s.s.Gui();
		ImGui::Begin("Entities");
		s.w.broadcast(SIGNAL_GUI, nullptr);
		ImGui::End();

		DoFadeInAnimation(fadein);

		rlImGuiEnd();
		EndDrawing();
	}

END:
	SaveGlobState();
	s.t.Unload();
	s.s.Unload();

	return restart;
}