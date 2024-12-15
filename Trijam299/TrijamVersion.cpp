#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>
#include <format>

static std::mt19937_64 rng;

#define MAP_WIDTH 16
#define MAP_HEIGHT 16

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
	Texture2D tileo;
	Texture2D tilec;
	Texture2D n1;
	Texture2D n2;
	Texture2D n3;
	Texture2D n4;
	Texture2D n5;
	Texture2D n6;
	Texture2D n7;
	Texture2D n8;
	Texture2D bomb;
	Texture2D flag;

	void Load() {
		tileo = LoadTexture("tileo.png");
		tilec = LoadTexture("tilec.png");
		n1 = LoadTexture("1.png");
		n2 = LoadTexture("2.png");
		n3 = LoadTexture("3.png");
		n4 = LoadTexture("4.png");
		n5 = LoadTexture("5.png");
		n6 = LoadTexture("6.png");
		n7 = LoadTexture("7.png");
		n8 = LoadTexture("8.png");
		bomb = LoadTexture("mine.png");
		flag = LoadTexture("flag.png");
	}

	void Unload() {
		UnloadTexture(flag);
		UnloadTexture(bomb);
		UnloadTexture(n8);
		UnloadTexture(n7);
		UnloadTexture(n6);
		UnloadTexture(n5);
		UnloadTexture(n4);
		UnloadTexture(n3);
		UnloadTexture(n2);
		UnloadTexture(n1);
		UnloadTexture(tilec);
		UnloadTexture(tileo);
	}
};

enum Tile {
	T_CLOSED,
	T_FLAGGED,
	T_OPEN,
	T_DEATH
};

enum StateType {
	S_MS_BEGIN,
	S_MS,
	S_MS_FAIL,
	S_MS_WIN,
	S_RAY
};

struct State {
	Textures t;
	flux::Group g;
	int tiles[MAP_HEIGHT][MAP_WIDTH];
	Tile tileState[MAP_HEIGHT][MAP_WIDTH];
	StateType stateType = S_MS_BEGIN;
	int mines = 0;
} s;

Texture2D *GetTexture(int level) {
	switch (level) {
	case 1:
		return &s.t.n1;
	case 2:
		return &s.t.n2;
	case 3:
		return &s.t.n3;
	case 4:
		return &s.t.n4;
	case 5:
		return &s.t.n5;
	case 6:
		return &s.t.n6;
	case 7:
		return &s.t.n7;
	case 8:
		return &s.t.bomb;
	}
	return nullptr;
}

void DrawTile(int x, int y) {
	int tile = s.tiles[y][x];
	Tile state = s.tileState[y][x];
	if (state == T_CLOSED) {
		DrawTexture(s.t.tilec, x * 24, y * 24, WHITE);
	}
	else if (state == T_FLAGGED) {
		DrawTexture(s.t.tilec, x * 24, y * 24, WHITE);
		DrawTexture(s.t.flag, x * 24, y * 24, WHITE);
	}
	else {
		Texture2D *tex = GetTexture(tile);
		if (state == T_DEATH)
			DrawRectangle(x * 24, y * 24, 24, 24, RED);
		else
			DrawTexture(s.t.tileo, x * 24, y * 24, WHITE);
		if (tex != nullptr) {
			DrawTexture(*tex, x * 24, y * 24, WHITE);
		}
	}
}

static bool GameOver() {
	StopSound(SND_MUSIC);
	const char *t = TextFormat("You made $!");
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText(t, (800 - MeasureText(t, 60)) / 2, 100, 60, WHITE);
		//DrawKeybindBar("", "");
		EndDrawing();
	}
	return false;
}

void LoadMap() {
	s.mines = 9 + rng() % 13;
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			s.tileState[y][x] = T_CLOSED;
			s.tiles[y][x] = 0;
		}
	}
	for (int i = 0; i < s.mines; i++) {
		s.tiles[rng() % MAP_HEIGHT][rng() % MAP_HEIGHT] = 8;
	}
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (s.tiles[y][x] == 8)
				continue;
			bool tl = false;
			bool t = false;
			bool tr = false;
			bool l = false;
			bool r = false;
			bool bl = false;
			bool b = false;
			bool br = false;
			if (x > 0) {
				if (y > 0)
					tl = s.tiles[y - 1][x - 1] == 8;
				l = s.tiles[y][x - 1] == 8;
				if (y < MAP_HEIGHT - 1)
					bl = s.tiles[y + 1][x - 1] == 8;
			}
			if (y > 0)
				t = s.tiles[y - 1][x] == 8;
			if (y < MAP_HEIGHT - 1)
				b = s.tiles[y + 1][x] == 8;
			if (x < MAP_WIDTH - 1) {
				if (y > 0)
					tr = s.tiles[y - 1][x + 1] == 8;
				r = s.tiles[y][x + 1] == 8;
				if (y < MAP_HEIGHT - 1)
					br = s.tiles[y + 1][x + 1] == 8;
			}
			s.tiles[y][x] = tl + t + tr + l + r + bl + b + br;
		}
	}
}

void CloseTiles() {
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			s.tileState[y][x] = T_CLOSED;
		}
	}
}

void OpenTileCorner(int x, int y) {
	if (x < 0 || y < 0)
		return;
	if (x >= MAP_WIDTH || y >= MAP_HEIGHT)
		return;
	if (s.tileState[y][x] != T_CLOSED)
		return;
	if (s.tiles[y][x] == 0)
		return;
	s.tileState[y][x] = T_OPEN;
}

void OpenTile(int x, int y) {
	if (x < 0 || y < 0)
		return;
	if (x >= MAP_WIDTH || y >= MAP_HEIGHT)
		return;
	if (s.tileState[y][x] != T_CLOSED)
		return;
	s.tileState[y][x] = T_OPEN;
	if (s.tiles[y][x] != 0)
		return;
	OpenTile(x + 1, y);
	OpenTile(x, y + 1);
	OpenTile(x - 1, y);
	OpenTile(x, y - 1);
	OpenTileCorner(x + 1, y + 1);
	OpenTileCorner(x - 1, y + 1);
	OpenTileCorner(x - 1, y - 1);
	OpenTileCorner(x + 1, y - 1);
}

void Flag(int x, int y) {
	if (x < 0 || y < 0)
		return;
	if (x >= MAP_WIDTH || y >= MAP_HEIGHT)
		return;
	if (s.tileState[y][x] != T_CLOSED && s.tileState[y][x] != T_FLAGGED)
		return;
	s.tileState[y][x] = s.tileState[y][x] == T_CLOSED ? T_FLAGGED : T_CLOSED;
}

bool WonSweeper() {
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (s.tiles[y][x] != 8 && s.tileState[y][x] != T_OPEN)
				return false;
		}
	}
	return true;
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	rng = std::mt19937_64(std::random_device()());
	CloseTiles();

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		int tX = GetMouseX() / 24;
		int tY = GetMouseY() / 24;

		if (s.stateType == S_MS_BEGIN) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				if (tX > 0 && tY > 0 && tX < MAP_WIDTH && tY < MAP_HEIGHT) {
					do {
						LoadMap();
					} while (s.tiles[tY][tX] != 0);
					OpenTile(tX, tY);
					s.stateType = S_MS;
				}
			}
		}
		else if (s.stateType == S_MS) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				Flag(tX, tY);
			}
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				OpenTile(tX, tY);
				if (s.tileState[tY][tX] == T_OPEN && s.tiles[tY][tX] == 8) {
					s.tileState[tY][tX] = T_DEATH;
					s.stateType = S_MS_FAIL;
				}
				if (WonSweeper()) {
					s.stateType = S_MS_WIN;
				}
			}
		}
		else if (s.stateType == S_MS_WIN) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				s.stateType = S_RAY;
			}
		}

		//PlaySound(SND_MUSIC);

		BeginDrawing();

		ClearBackground(GRAY);

		if (s.stateType != S_RAY) {
			for (int y = 0; y < MAP_HEIGHT; y++) {
				for (int x = 0; x < MAP_WIDTH; x++) {
					DrawTile(x, y);
				}
			}
		}

		if (s.stateType == S_MS_FAIL) {
			const char *tT = "Retrieval Failure";
			const char *tB = "Refresh to play again";
			DrawRectangle(0, 0, SCRWID, SCRHEI, Fade(BLACK, 0.5f));
			DrawText(tT, (SCRWID - MeasureText(tT, 30)) / 2, 20, 30, WHITE);
			DrawText(tB, (SCRWID - MeasureText(tB, 20)) / 2, SCRHEI - 31 - 20 - 20, 20, WHITE);
		}
		else if (s.stateType == S_MS_WIN) {
			const char *tT = "Mines Swept";
			const char *tM = TextFormat("%d mines reclaimed", s.mines);
			const char *tB = "Copyright Raytheon";
			DrawRectangle(0, 0, SCRWID, SCRHEI, Fade(BLACK, 0.5f));
			DrawText(tT, (SCRWID - MeasureText(tT, 30)) / 2, 20, 30, WHITE);
			DrawText(tM, (SCRWID - MeasureText(tM, 20)) / 2, (SCRHEI - 20) / 2, 20, WHITE);
			DrawText(tB, (SCRWID - MeasureText(tB, 20)) / 2, SCRHEI - 31 - 20 - 20, 20, WHITE);
		}
		
		if (s.stateType == S_MS_BEGIN) {
			DrawKeybindBar("[Left Click] Begin", "", false);
		}
		else if (s.stateType == S_MS) {
			DrawKeybindBar("[Left Click] Sweep [Right Click] Flag", TextFormat("Mines: %d", s.mines), false);
		}
		else if (s.stateType == S_MS_FAIL) {
			DrawKeybindBar("", "", false);
		}
		else if (s.stateType == S_MS_WIN) {
			DrawKeybindBar("[Left Click] Continue...", "", false);
		}
		else if (s.stateType == S_RAY) {
			DrawKeybindBar("[Left Click] Drop", "", false);
		}

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();

	//StopSound(SND_MUSIC);

	return restart;
}