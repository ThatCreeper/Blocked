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
	S(potion) \
	S(spotroom) \
	S(hitler) \
	S(baby)
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
const char *potionNames[] = {
	"Soap potion",
	"Glass potion",
	"Mushroom Juice",
	"Hot Chocolate",
	"Washed Glass potion",
	"Clean Mushrooms Potion",
	"Gross Chocolate Potion",
	"Chopped Mushrooms Potion",
	"Crushed Chocolate Potion",
	"Chocolate covered mushrooms potion"
};

struct texter : entity {
	const char *text_;

	texter(const char *text) : entity(), text_(text) {}

	void render() override {
		ClearBackground(BLACK);
		DrawText(text_, 10, 10, 30, WHITE);
	}
};

struct positional : entity {
	float x = 0;
	float y = 0;
	float scale = 1.0;

	void gui() override {
		if (guiHeader("positional")) {
			if (ImGui::Button("kill")) w->remove(this);
			ImGui::DragFloat("x", &x);
			ImGui::DragFloat("y", &y);

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
		Color c = isOver() ? YELLOW : WHITE;
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
			DrawTexture(s.t.chocolate, x, y, c);
		}
	}
};

struct tex : positional {
	Texture2D *a_;
	Texture2D *b_;
	bool flip = false;
	float rotation = 0.0;

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
			ImGui::DragFloat("rotation", &rotation);
			ImGui::TreePop();
		}
	}

	void render() override {
		if (flip) DrawTextureEx(*a_, { (float)x, (float)y }, rotation, scale, WHITE);
		else DrawTextureEx(*b_, { (float)x, (float)y }, rotation, scale, WHITE);
		flip = !flip;
	}
};

void applyPotion(int p) {
	if (p == 0) { s.smell--; }
	if (p == 1) { s.health--; }
	if (p == 2) { s.strength++; }
	if (p == 3) { s.health--; s.strength--; }
	if (p == 4) { s.smell++; }
	if (p == 5) { s.strength++; s.health++; }
	if (p == 6) { s.health--; s.smell--; }
	if (p == 7) { s.strength++; s.smell++; }
	if (p == 8) { s.health--; s.smell++; }
	if (p == 9) { s.health--; s.strength; }
}

struct spotAnimator : entity {
	tex *h_;
	tex *f_;
	tex *i_;
	tex *f1_;
	tex *f2_;
	tex *f3_;
	tex *s_;
	tex *hit_;

	void update() override {
		StopSound(SND_WOOFARF);
	}

	void init() override {
		w->add(h_ = new tex(s.t.house));
		w->add(f_ = new tex(s.t.fire1, s.t.fire2));
		f_->x = 68;
		f_->y = 3;
		w->add(s_ = new tex(s.t.spot));
		s_->x = 851;
		s_->y = 321;

		flux::to(2)
			->with(&s_->x, 416)
			->oncomplete([this]
				{
					if (s.strength <= -2) {
						w->add(new texter("You win!\nSpot was too weak to enter\nthe house :)\nhe was a weak spot\n\nreload to play again"));
						PlaySound(SND_WIN);
					}
					else if (s.health <= -2) {
						flux::to(4)
							->with(&s_->y, -178)
							->oncomplete([this]
								{
									w->add(new texter("spot died (too weak)\n\nhow could you :(\n\nreload to play again"));
									PlaySound(SND_GAME_OVER);
								});
					}
					else {
						PlaySound(SND_EXPLOSION);
						w->add(h_ = new tex(s.t.inside));
						w->add(s_ = new tex(s.t.spot));
						s_->x = 836;
						s_->y = 254;
						flux::to(2)
							->with(&s_->x, 250)
							->oncomplete([]()
								{
									PlaySound(SND_SNIFF);
								})
							->after(2)
							->with(&s_->rotation, 360)
							->after(2)
							->oncomplete([this]
								{
									if (s.smell < -2)
									{
										if (s.strength >= 0) {
											flux::to(4)
												->with(&s_->x, -468.000)
												->oncomplete([this]
													{
														w->add(new texter("spot could not find the baby (too weak)\n\nyou win\n\ncongrats you made a weak spot\n\nreload to play again"));
														PlaySound(SND_WIN);
													});
										}
										else {
											flux::to(2)
												->oncomplete([this]
													{
														w->add(new texter("Your dog spot could not find the child\nnor could your dog escape the house\n\nyour beautiful dog has died\n\nreload to play again"));
														PlaySound(SND_GAME_OVER);
													});
										}
									}
									else {
										flux::to(2)
											->with(&s_->x, 54)
											->after(1)
											->with(&s_->scale, 0)
											->oncomplete([this]
												{
													PlaySound(SND_WAHWAHHITLER);
													w->add(h_ = new tex(s.t.hitler));
												})
											->after(4)
											->oncomplete([this]
												{
													if (s.strength < -1) {
														w->add(new texter("Spot & baby hitler both perished\nin the flames.\n\nYou weakened spot too much.\n\nreload to play again"));
														PlaySound(SND_GAME_OVER);
													}
													else {
														w->add(h_ = new tex(s.t.house));
														w->add(f_ = new tex(s.t.fire1, s.t.fire2));
														f_->x = 68;
														f_->y = 3;
														w->add(s_ = new tex(s.t.spot));
														s_->x = 600;
														s_->y = 321;
														w->add(hit_ = new tex(s.t.baby));

														w->add(new texter("Baby hitler was saved.\n\nYOU LOST!!!!!\n\nreload to play again"));
														PlaySound(SND_GAME_OVER);
													}
												});
									}
								});
					}
				});
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
			DrawText(potionNames[s.potion[s.potions - 1]], 180, 107, 20, BLACK);
			DrawText(potionNames[s.potion[s.potions - 1]], 180, 105, 20, WHITE);
		}
	}
};

struct spotRoom : entity {
	bool used[3] = { false, false, false };

	void init() override {
		//w->add(new tex(s.t.potion));
	}
	
	void update() override {
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			if (isOver(0)) { used[0] = true; applyPotion(s.potion[0]); }
			if (isOver(1)) { used[1] = true; applyPotion(s.potion[1]); }
			if (isOver(2)) { used[2] = true; applyPotion(s.potion[2]); }
		}

		if (IsKeyPressed(KEY_ENTER)) {
			w->remove(this);
			w->add(new spotAnimator);
		}
	}

	bool isOver(int n) {
		if (GetMouseY() >= 150) return false;
		if (GetMouseX() >= 320 + 150) return false;
		if (GetMouseX() >= 159 + 150) return n == 2;
		if (GetMouseX() >= 150) return n == 1;
		return n == 0;
	}

	void render() override {
		DrawTexture(s.t.spotroom, 0, 0, WHITE);
		if (!used[0] && isOver(0)) DrawRectangle(0, 0, 150, 150, YELLOW);
		if (!used[0]) DrawTexture(s.t.potion, 0, 0, potionColors[s.potion[0]]);
		if (!used[1] && isOver(1)) DrawRectangle(159, 0, 150, 150, YELLOW);
		if (!used[1]) DrawTexture(s.t.potion, 159, 0, potionColors[s.potion[1]]);
		if (!used[2] && isOver(2)) DrawRectangle(320, 0, 150, 150, YELLOW);
		if (!used[2]) DrawTexture(s.t.potion, 320, 0, potionColors[s.potion[2]]);
		const char *txt = TextFormat("%s\n%s\n%s", potionNames[s.potion[0]], potionNames[s.potion[1]], potionNames[s.potion[2]]);
		DrawText(txt, 0, 169, 20, BLACK);
		DrawText(txt, 0, 167, 20, WHITE);

		DrawKeybindBar("[Enter] Continue", "");
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

		/*s.t.Gui();
		s.s.Gui();
		ImGui::Begin("Entities");
		s.w.forEach<entity>([](entity *e) { e->gui(); });
		ImGui::End();
		s.gui();*/

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
	//if (isUsed()) return;
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isOver()) {
		if (s.sels == 0) {
			s.sels++;
			s.selA = idx_;
		}
		else if (s.sels == 1) {
			s.sels++;
			s.selB = idx_;

			int p = 0;
			if (isCombo(2, 2)) { p = 0; }
			if (isCombo(0, 0)) { p = 1; }
			if (isCombo(1, 1)) { p = 2; }
			if (isCombo(3, 3)) { p = 3; }
			if (isCombo(0, 2)) { p = 4; }
			if (isCombo(1, 2)) { p = 5; }
			if (isCombo(2, 3)) { p = 6; }
			if (isCombo(0, 1)) { p = 7; }
			if (isCombo(0, 3)) { p = 8; }
			if (isCombo(1, 3)) { p = 9; }
			s.potion[s.potions] = p;
			s.potions++;

			flux::to(1)
				->oncomplete([this]
					{
						s.sels = 0;
						if (s.potions == 3) {
							s.w.add(new spotRoom);
						}
					});
		}
	}
}
