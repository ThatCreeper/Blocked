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

struct Textures {

	void Load() {
	}

	void Unload() {
	}
};

struct Map {
	int wid;
	int hei;
};

const int mapdata[] = {
	// Map 0
	-1, -2,  2, -3,
	 1,  1,  2,  3,
	-2, -1,  2,  3,
	 2,  2,  2, -3,

	// Map 1
	-1, -2,  2, -3, -4,
	 1, -5, -2,  3,  4,
	 1,  5, -5, -3,  4,
	 1,  1,  1, -1,  4,
	-4,  4,  4,  4,  4,

	// Map 2
	 2,  2,  2,  3,  3,  3,
	-2, -1,  2, -3, -2,  3,
	-6,  1,  2,  2,  2,  3,
	 6,  1,  1,  1, -1,  3,
	 6, -6, -4,  4, -3,  3,
	-5,  5, -5,  4,  4, -4,

	// Map 3
	-2, -1,  2,  2,  2,  2,  2,
	 2,  1,  2,  1,  1,  1,  2,
	 2,  1,  2,  1, -2,  1,  2,
	 2,  1,  2, -1,  2,  1,  2,
	 2,  1,  2,  2,  2,  1,  2,
	 2,  1,  1,  1,  1,  1,  2,
	 2,  2,  2,  2,  2,  2,  2,

	// Map 4
	-3,  4, -4, -2,  2,  7, -7,
	 3, -4, -1,  1,  2,  7, -5,
	 3,  3,  3,  1,  2, -7,  5,
	-5, -2,  3,  1,  2,  5,  5,
	 5,  2, -3, -1,  2,  5, -6,
	 5,  2,  2,  2,  2,  5,  6,
	 5,  5,  5,  5,  5,  5, -6,

	// Map 5
	-2,  2, -4,  4,  4, -4,
	-1,  2, -1, -3,  3,  3,
	 1,  2,  1,  1,  1,  3,
	 1,  2,  2, -2,  1,  3,
	 1,  1,  1,  1,  1, -3,

	// Map 6
	-6, -1, -5,  5,  5,  5, -5,
	 6,  1,  1, -1, -3,  3,  3,
	 6,  6,  6,  6,  4, -4,  3,
	 7,  7,  7,  6,  4, -2,  3,
	 7, -8,  7,  6,  4,  2,  3,
	 7,  8,  7, -6, -4,  2, -3,
	-7, -8,  7,  7, -7,  2, -2,

	// Map 7
	 4,  4, -4, -5,  5,  5,  5,  8, -8,
	 4,  2,  2,  2,  6, -6,  5,  8, -7,
	 4,  2, -1,  2,  6, -5,  5, -8,  7,
	 4,  2,  1,  2,  6,  6,  6,  6,  7,
	 4,  2,  1,  2,  2,  2,  2,  6,  7,
	 4,  2,  1,  1,  1,  1,  2,  6,  7,
	 4,  2,  2,  2,  2,  1,  2,  6,  7,
	 4, -3,  3,  3, -2,  1,  2, -6,  7,
	 4,  4, -4,  3, -3, -1,  2, -2, -7,
};

const static Map maps[] = {
	{
		4, 4,
	},
	{
		5, 5
	},
	{
		6, 6
	},
	{
		7, 7
	},
	{
		7, 7
	},
	{
		6, 5
	},
	{
		7, 7
	},
	{
		9, 9
	}
};

struct State {
	Textures t;
	flux::Group g;
	int m;
	int *f;
	int p;
	Camera2D c;
	bool froz = false;
	float timeTillNext = 1;
	float opac = 0;
} s;

int getdata(int x, int y) {
	if (x < 0)
		return 0;
	if (y < 0)
		return 0;
	if (x >= maps[s.m].wid)
		return 0;
	if (y >= maps[s.m].hei)
		return 0;
	int S = 0;
	for (int i = 0; i < s.m; i++) {
		S += maps[i].wid * maps[i].hei;
	}
	return mapdata[S + y * maps[s.m].wid + x];
}

int mapwid() {
	return maps[s.m].wid;
}
int maphei() {
	return maps[s.m].hei;
}

static bool GameOver() {
	StopSound(SND_MUSIC);
	const char *t = TextFormat("That's all for now...");
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText(t, (800 - MeasureText(t, 60)) / 2, 100, 60, WHITE);
		//DrawKeybindBar("", "");
		EndDrawing();
	}
	return false;
}

bool checkwin() {
	for (int i = 0; i < maphei(); i++) {
		for (int j = 0; j < mapwid(); j++) {
			int d = getdata(j, i);
			if (d > 0 && s.f[i * mapwid() + j] != d)
				return false;
		}
	}
	return true;
}

bool nextmap() {
	s.m++;
	if (s.m >= sizeof(maps) / sizeof(*maps)) {
		return true;
	}
	if (s.f)
		delete[] s.f;
	s.f = new int[mapwid() * maphei()];
	memset(s.f, 0, sizeof(int) * mapwid() * maphei());
	s.p = 0;
	s.froz = false;
	s.timeTillNext = 1;
	s.c.offset.x = GetRenderWidth() / 2;
	s.c.offset.y = GetRenderHeight() / 2;
	s.c.target.x = mapwid() * 32 / 2;
	s.c.target.y = maphei() * 32 / 2;
	return false;
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	rng = std::mt19937_64(0x007ABCD);
	s.m = -1;
	s.c = { 0 };
	s.c.zoom = 2;
	nextmap();

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		s.c.offset.x = GetRenderWidth() / 2;
		s.c.offset.y = GetRenderHeight() / 2;
		s.c.target.x = mapwid() * 32 / 2;
		s.c.target.y = maphei() * 32 / 2;

		Vector2 m = GetScreenToWorld2D(GetMousePosition(), s.c);
		int mtx = m.x / 32;
		int mty = m.y / 32;
		mtx = Clamp(mtx, 0, mapwid() - 1);
		mty = Clamp(mty, 0, maphei() - 1);

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			s.p = getdata(mtx, mty);
			if (s.p < 0) {
				s.p = -s.p;
				for (int i = 0; i < maphei(); i++) {
					for (int j = 0; j < mapwid(); j++) {
						if (s.f[i * mapwid() + j] == s.p)
							s.f[i * mapwid() + j] = 0;
					}
				}
			}
			else {
				s.p = s.f[mty * mapwid() + mtx];
			}
		}

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			if (!s.froz && s.p != 0 && getdata(mtx, mty) > 0)
				s.f[mty * mapwid() + mtx] = s.p;
		}

		if (checkwin()) {
			s.froz = true;
			s.timeTillNext -= GetFrameTime();
			if (s.timeTillNext <= 0)
				if (nextmap()) {
					restart = GameOver();
					goto END;
				}
		}

		PlaySound(SND_MUSIC);

		BeginDrawing();

		ClearBackground(BLACK);

		BeginMode2D(s.c);

		DrawRectangle(0, 0, mapwid() * 32, maphei() * 32, DARKGRAY);

		for (int i = 0; i < maphei(); i++) {
			for (int j = 0; j < mapwid(); j++) {
				Color colors[] = {
					RED,
					GREEN,
					BLUE,
					PURPLE,
					ORANGE,
					YELLOW,
					PINK,
					{ 33, 190, 160, 255 },
					MAROON
				};
				int r = s.f[i * mapwid() + j];
				if (r == 0)
					continue;
				Color c = ColorBrightness(colors[r], -0.2f);
				DrawCircle(j * 32 + 16, i * 32 + 16, 12, c);
				bool t = i > 0 && (getdata(j, i - 1) == -r || s.f[(i - 1) * mapwid() + j] == r);
				bool l = j > 0 && (getdata(j - 1, i) == -r || s.f[(i) * mapwid() + j - 1] == r);
				bool R = j < maps[s.m].wid - 1 && (getdata(j + 1, i) == -r || s.f[(i) * mapwid() + j + 1] == r);
				bool b = i < maps[s.m].hei - 1 && (getdata(j, i + 1) == -r || s.f[(i + 1)*mapwid() + j] == r);
				if (t)
					DrawRectangle(j * 32 + 16 - 12, i * 32 - 16, 24, 32, c);
				if (b)
					DrawRectangle(j * 32 + 16 - 12, i * 32 + 16, 24, 32, c);
				if (l)
					DrawRectangle(j * 32 - 16, i * 32 + 16 - 12, 32, 24, c);
				if (R)
					DrawRectangle(j * 32 + 16, i * 32 + 16 - 12, 32, 24, c);
			}
		}


		for (int i = 0; i < maphei(); i++) {
			for (int j = 0; j < mapwid(); j++) {
				Color colors[] = {
					RED,
					GREEN,
					BLUE,
					PURPLE,
					ORANGE,
					YELLOW,
					PINK,
					{ 33, 190, 160, 255 },
					MAROON
				};
				int r = getdata(j, i);
				bool h = r < 0;
				if (!h) {
					continue;
				}
				DrawCircle(j * 32 + 16, i * 32 + 16, 12, colors[-r]);
			}
		}

		if (!s.froz) {
			DrawRectangleLines(mtx * 32, mty * 32, 32, 32, RED);
			s.opac = Lerp(s.opac, 0, GetFrameTime(), 0.2);
		}
		else {
			s.opac = 1;
		}
		DrawText("Nice!", s.c.target.x - MeasureText("Nice!", 30) / 2, s.c.target.y - 15, 30, Fade(WHITE, s.opac));

		EndMode2D();

		DrawKeybindBar("[Click] Solve", "");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	delete[] s.f;
	s.t.Unload();
	

	StopSound(SND_MUSIC);

	return restart;
}