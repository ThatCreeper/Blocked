#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>
#include <format>

static std::mt19937_64 rng;
static constexpr Color BGBLUE = { 0x66, 0x9b, 0xf7, 0xff };

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
	Image dataO;
	Texture2D map;
	Texture2D mapO;
	Texture2D tools;
	Texture2D energy;
	Texture2D tile;

	void Load() {
		dataO = LoadImage("mapO.png");
		map = LoadTexture("map.png");
		mapO = LoadTextureFromImage(dataO);
		tools = LoadTexture("tools.png");
		energy = LoadTexture("energy.png");
		tile = LoadTexture("tile.png");
	}

	void Unload() {
		UnloadTexture(tile);
		UnloadTexture(energy);
		UnloadTexture(tools);
		UnloadTexture(mapO);
		UnloadTexture(map);
		UnloadImage(dataO);
	}
};

struct PowerPlant {
	float x;
	float y;
	float remaining;
	bool decommisioned;
};

struct State {
	Textures t;
	flux::Group g;
	bool glasses = true;
	bool gOn = false;
	int selection = 0;
	float producedEnergy = 0;
	float usedEnergy = 0;
	int m = 1000;
	float mSize = 1;
	float usedSqrPx = 0;
	bool gainedM = true;
	std::vector<PowerPlant> powerPlants;
	Camera2D cam;
	std::string taxString = "";
} s;

static bool GameOver() {
	StopSound(SND_MUSIC);
	const char *t = TextFormat("You made $%d!", s.m);
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText(t, (800 - MeasureText(t, 60)) / 2, 100, 60, WHITE);
		//DrawKeybindBar("", "");
		EndDrawing();
	}
	return false;
}

Color GetPixelColor(int x, int y) {
	if (x < 0 || y < 0)
		return BLACK;
	if (x >= s.t.dataO.width)
		return BLACK;
	if (y >= s.t.dataO.height)
		return BLACK;
	return GetImageColor(s.t.dataO, x, y);
}

bool MouseOverMenu() {
	return GetMouseX() <= 20 * 3;
}

const char *CurSelected() {
	switch (s.selection) {
	case 0:
		return "Coal Power Generation";
	case 1:
		return "Fracker";
	case 2:
		return "Solar Array";
	}
	return "??";
}

int GetCurWidth() {
	switch (s.selection) {
	case 0:
		return 230;
	case 1:
		return 80;
	case 2:
		return 90;
	}
	return 0;
}

int GetCurHeight() {
	switch (s.selection) {
	case 0:
		return 150;
	case 1:
		return 80;
	case 2:
		return 90;
	}
	return 0;
}

int GetCurCost() {
	switch (s.selection) {
	case 0:
		return 400;
	case 1:
		return 200;
	case 2:
		return 600;
	}
	return 999999;
}

void GetHovering(PowerPlant **fracker) {
	*fracker = nullptr;
	for (PowerPlant &f : s.powerPlants) {
		if (CheckCollisionPointRec(GetScreenToWorld2D(GetMousePosition(), s.cam), { f.x, f.y, 230, 150 })) {
			*fracker = &f;
			break;
		}
	}
}

bool IsHovering() {
	PowerPlant *f;
	GetHovering(&f);
	return f != nullptr;
}

bool IsLegalPlacement() {
	Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), s.cam);

	if (GetPixelColor(mouse.x - GetCurWidth() / 2, mouse.y - GetCurHeight() / 2).r != 255 ||
		GetPixelColor(mouse.x + GetCurWidth() / 2, mouse.y + GetCurHeight() / 2).r != 255)
		return false;

	Rectangle r = { mouse.x - GetCurWidth() / 2, mouse.y - GetCurHeight() / 2, GetCurWidth(), GetCurHeight() };
	for (PowerPlant &f : s.powerPlants) {
		if (CheckCollisionRecs(r, { f.x, f.y, 230, 150 }))
			return false;
	}
	return true;
}

void MoneyShrink() {
	s.mSize = 0.9;
	s.gainedM = false;
	s.g.to(0.3f)
		->with(&s.mSize, 1);
}

void PlacePowerPlant() {
	if (!IsLegalPlacement())
		return;
	if (s.m <= 400)
		return;
	s.m -= 400;
	MoneyShrink();
	Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), s.cam);
	PowerPlant f;
	f.x = mouse.x - GetCurWidth() / 2;
	f.y = mouse.y - GetCurHeight() / 2;
	f.remaining = GetPixelColor(mouse.x, mouse.y).g == 255 ? 240 : 120;
	f.decommisioned = false;
	s.powerPlants.push_back(f);
	s.producedEnergy += 10;
	s.usedSqrPx += 230 * 150;
}

void Place() {
	switch (s.selection) {
	case 0:
		PlacePowerPlant();
	}
}

void Decommission() {
	PowerPlant *f;
	GetHovering(&f);
	if (f != nullptr && !f->decommisioned) {
		f->decommisioned = true;
		s.usedEnergy -= 10;
	}
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	rng = std::mt19937_64(0x007ABCD);
	float tileY = 40;
	float timeUntilTax = 0;

	s.cam = { 0 };
	s.cam.offset.x = SCRWID / 2;
	s.cam.offset.y = SCRHEI / 2;
	s.cam.target.x = s.t.dataO.width / 2;
	s.cam.target.y = s.t.dataO.height / 2;
	s.cam.zoom = 1;

	Vector2 lastMouse = GetScreenToWorld2D(GetMousePosition(), s.cam);

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), s.cam);

		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		bool validPlacement = false;

		if (MouseOverMenu()) {
			int mx = GetMouseX();
			int my = GetMouseY();
			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
				if (my >= 40 + 16 * 0 * 3 && my < 40 + 16 * 1 * 3)
					s.selection = 0;
				else if (my >= 40 + 16 * 1 * 3 && my < 40 + 16 * 2 * 3)
					s.selection = 1;
				else if (my >= 40 + 16 * 2 * 3 && my < 40 + 16 * 3 * 3)
					s.selection = 2;
			}
		}
		else {
			s.cam.zoom += GetMouseWheelMove() * 0.1f;
			s.cam.zoom = Clamp(s.cam.zoom, 0.2f, 3.f);
			if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
				s.cam.target.x += lastMouse.x - mouse.x;
				s.cam.target.y += lastMouse.y - mouse.y;
			}
			mouse = GetScreenToWorld2D(GetMousePosition(), s.cam);
			lastMouse = mouse;

			if (s.glasses && IsKeyPressed(KEY_SPACE))
				s.gOn = !s.gOn;

			if (IsKeyPressed(KEY_ONE))
				s.selection = 0;
			if (IsKeyPressed(KEY_TWO))
				s.selection = 1;
			if (IsKeyPressed(KEY_THREE))
				s.selection = 2;

			if (IsKeyPressed(KEY_X))
				Decommission();
			validPlacement = IsLegalPlacement();
			if (validPlacement && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				Place();
		}

		timeUntilTax -= GetFrameTime();
		if (timeUntilTax <= 0) {
			timeUntilTax = 4;
			int plants = 0;
			bool taxed = false;
			for (PowerPlant &p : s.powerPlants) {
				if (!p.decommisioned)
					plants++;
			}
			if (plants > 0) {
				taxed = true;
				int t = 5 * plants;
				s.taxString = std::format("Fined ${} for pollution", t);
				s.m -= t;
			}
			if (taxed)
				MoneyShrink();
		}

		if (s.m <= 0) {
			restart = GameOver();
			goto END;
		}

		tileY = LerpPixelRound(tileY, 40 + 16 * 3 * s.selection, GetFrameTime(), 0.05f, 16.0f / 3.0f);

		//PlaySound(SND_MUSIC);

		BeginDrawing();

		ClearBackground(s.gOn ? BLACK : BGBLUE);
		
		BeginMode2D(s.cam);

		if (s.gOn) {
			DrawTexture(s.t.mapO, 0, 0, WHITE);
		}
		else {
			DrawTexture(s.t.map, 0, 0, WHITE);
		}

		for (PowerPlant f : s.powerPlants) {
			DrawRectangle(f.x, f.y, 230, 150, f.decommisioned ? BLACK : PURPLE);
		}

		if (!MouseOverMenu()) {
			rlSetLineWidth(3);
			DrawRectangleLines(mouse.x - GetCurWidth() / 2, mouse.y - GetCurHeight() / 2, GetCurWidth(), GetCurHeight(), validPlacement ? GREEN : RED);
		}

		EndMode2D();
		
		DrawTextPro(GetFontDefault(), TextFormat("$%d", s.m), { 5, 5 }, { 5, 5 }, (-s.mSize + 1) * 40, 30 * s.mSize, 3 * s.mSize, s.gainedM ? GREEN : RED);
		DrawText(s.taxString.c_str(), 10 + 16 * 3, 40, 20, RED);

		DrawTextureEx(s.t.tools, { 0, 30 + 5 + 5 }, 0, 3, WHITE);
		DrawTextureEx(s.t.tile, { 0, tileY }, 0, 3, WHITE);

		DrawTexture(s.t.energy, SCRWID - 64 - 5, 5, WHITE);
		{
			const char *e = TextFormat("%.0f", s.producedEnergy - s.usedEnergy);
			DrawText(e, SCRWID - 64 - 5 - MeasureText(e, 30) / 2, 74, 30, YELLOW);
		}

		DrawKeybindBar(IsHovering() ? "[1] [2] [3] [Click] [X] Decomission" : TextFormat("[1] [2] [3] [Click] Use %s ($%d)", CurSelected(), GetCurCost()), s.glasses ? "[Space] [RClick] [Scroll]" : "[Right Click] [Scroll]");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();

	//StopSound(SND_MUSIC);

	return restart;
}