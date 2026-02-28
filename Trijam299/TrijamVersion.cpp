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
	T(baselut, "baselut.png") \
	S(house) \
	S(fire1) \
	S(fire2) \
	S(spot) \
	S(spot_wings) \
	S(inside) \
	S(glass_shards) \
	S(mushrooms) \
	S(potion_room) \
	S(soap) \
	S(chocolate) \
	S(potion)
#define S(a) T(a, #a ".png")
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
#undef S
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

	float health = 0.0;
	float smell = 0.0;
	float strength = 0.0;

	int selA = -1;
	int selB = -1;
	int sels = 0;

	int potion[3] = { -1, -1, -1 };
	int potions = 0;

	void gui() {
		ImGui::Begin("State");
		ImGui::InputFloat("health", &health);
		ImGui::InputFloat("smell", &smell);
		ImGui::InputFloat("strength", &strength);
		ImGui::InputInt("selA", &selA);
		ImGui::InputInt("selB", &selB);
		ImGui::InputInt("sels", &sels);
		ImGui::InputInt("potionA", &potion[0]);
		ImGui::InputInt("potionB", &potion[1]);
		ImGui::InputInt("potionC", &potion[2]);
		ImGui::InputInt("potions", &potions);
		ImGui::End();
	}
} s;

Color potionColors[] = {
	{72, 143, 70, 255},
	{199, 242, 233, 255},
	{137, 143, 70, 255},
	{64, 39, 15, 255},
	{119, 191, 188, 255},
	{214, 187, 148, 255},
	{104, 107, 25, 255},
	{122, 107, 135, 255},
	{122, 107, 135, 255},
	{136, 116, 148, 255}
};

struct positional : entity {
	int x = 0;
	int y = 0;
	float scale = 1.0;

	void gui() override {
		if (guiHeader("positional")) {
			if (ImGui::Button("kill")) w->remove(this);
			ImGui::DragInt("x", &x);
			ImGui::DragInt("y", &y);

			ImGui::TreePop();
		}
	}

	entity *positioned(int x_, int y_) {
		x = x_;
		y = y_;
		return this;
	}
};

struct rectRend : positional {
	Color color_;
	int wid_;
	int hei_;
	int offX_;
	int offY_;

	rectRend(Color color, int wid, int hei, int offX, int offY) : positional() {
		color_ = color;
		wid_ = wid;
		hei_ = hei;
		offX_ = offX;
		offY_ = offY;
	}

	void render() override {
		DrawRectangle(x + offX_, y + offY_, wid_, hei_, color_);
	}
};

struct ingredient : positional {
	int idx_ = 0;

	ingredient(int idx) {
		idx_ = idx;
	}

	void gui() override {
		if (guiHeader("ingredient")) {
			positional::gui();
			ImGui::TreePop();
		}
	}

	void init() override {
		if (idx_ >= 2) {
			x = SCRWID - 150 - 20;
			y = (idx_ - 1) * SCRHEI / 3 - 75;
		}
		else {
			x = 20;
			y = (idx_ + 1) * SCRHEI / 3 - 75;
		}
	}

	bool isOver() {
		return GetMouseX() >= x && GetMouseY() >= y && GetMouseX() < x + 150 && GetMouseY() < y + 150;
	}

	void update() override;

	bool isUsed() {
		return (s.sels >= 1 && s.selA == idx_) ||
			(s.sels >= 2 && s.selB == idx_);
	}

	void render() override {
		Color c = isUsed() ? GRAY : isOver() ? YELLOW : WHITE;
		if (idx_ == 0) {
			DrawTexture(s.t.glass_shards, x, y, c);
		}
		else if (idx_ == 1) {
			DrawTexture(s.t.mushrooms, x, y, c);
		}
		else if (idx_ == 2) {
			DrawTexture(s.t.soap, x, y, c);
		}
		else {
			DrawRectangle(x, y, 150, 150, c);
		}
	}
};

struct tex : positional {
	Texture2D *a_;
	Texture2D *b_;
	bool flip = false;

	tex(Texture2D &a, Texture2D &b) {
		a_ = &a;
		b_ = &b;
	}
	tex(Texture2D &a) {
		a_ = &a;
		b_ = &a;
	}

	void gui() override {
		if (guiHeader("tex")) {
			positional::gui();
			ImGui::DragFloat("scale", &scale);
			ImGui::TreePop();
		}
	}

	void render() override {
		if (flip) DrawTextureEx(*a_, { (float)x, (float)y }, 0, scale, WHITE);
		else DrawTextureEx(*b_, { (float)x, (float)y }, 0, scale, WHITE);
		flip = !flip;
	}
};

struct spotAnimator : entity {
	tex *h_;
	tex *f_;
	tex *i_;
	tex *f1_;
	tex *f2_;
	tex *f3_;

	void init() override {
		w->add(h_ = new tex(s.t.house));
		w->add(f_ = new tex(s.t.fire1, s.t.fire2));
		f_->x = 68;
		f_->y = 3;
		w->add(i_ = new tex(s.t.inside));
		w->add(f1_ = new tex(s.t.fire1, s.t.fire2));
		f1_->x = -118;
		f1_->y = 9;
		w->add(f2_ = new tex(s.t.fire1, s.t.fire2));
		f2_->x = -223;
		f2_->y = 172;
		w->add(f3_ = new tex(s.t.fire1, s.t.fire2));
		f3_->x = 280;
		f3_->y = 158;
	}

	void onRemove() override {
		if (h_) w->remove(h_);
		if (f_) w->remove(f_);
	}
};

struct craftVis : entity {
	Texture2D getTex(int idx) {
		if (idx == 0) {
			return s.t.glass_shards;
		}
		else if (idx == 1) {
			return s.t.mushrooms;
		}
		else if (idx == 2) {
			return s.t.soap;
		}
		else {
			return s.t.chocolate;
		}
	}

	void render() override {
		if (s.sels >= 1) {
			DrawTextureEx(getTex(s.selA), { 188, 18 }, 0, 0.5, WHITE);
		}
		if (s.sels >= 2) {
			DrawTextureEx(getTex(s.selB), { 354, 18 }, 0, 0.5, WHITE);
			DrawTextureEx(s.t.potion, { 518, 18 }, 0, 0.5, potionColors[s.potion[s.potions - 1]]);
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

	s.w.add(new tex(s.t.potion_room));
	s.w.add(new ingredient(0));
	s.w.add(new ingredient(1));
	s.w.add(new ingredient(2));
	s.w.add(new ingredient(3));
	s.w.add(new craftVis);

	s.w.add(new tex(s.t.mushrooms));

	while (!WindowShouldClose()) {
		PlaySound(SND_WOOFARF);

		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		s.w.update();

		BeginTextureMode(render);

		ClearBackground(BLACK);

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
		s.w.forEach<entity>([](entity *e) { e->gui(); });
		ImGui::End();
		s.gui();

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

bool isCombo(int a, int b) {
	return (s.selA == a && s.selB == b) || (s.selA == b && s.selB == a);
}

inline void ingredient::update() {
	positional::update();
	if (isUsed()) return;
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isOver()) {
		if (s.sels == 0) {
			s.sels++;
			s.selA = idx_;
		}
		else if (s.sels == 1) {
			s.sels++;
			s.selB = idx_;

			int p = 0;
			if (isCombo(2, 2)) p = 0;
			if (isCombo(0, 0)) p = 1;
			if (isCombo(1, 1)) p = 2;
			if (isCombo(3, 3)) p = 3;
			if (isCombo(0, 2)) p = 4;
			if (isCombo(1, 2)) p = 5;
			if (isCombo(2, 3)) p = 6;
			if (isCombo(0, 1)) p = 7;
			if (isCombo(0, 3)) p = 8;
			if (isCombo(1, 3)) p = 9;
			s.potion[s.potions] = p;
			s.potions++;

			flux::to(1)
				->oncomplete([this]
					{
						s.sels = 0;
						if (s.potions == 3) {
							s.w.add(new spotAnimator);
						}
					});
		}
	}
}
