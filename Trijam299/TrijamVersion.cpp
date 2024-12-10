#include "global.h"
#include <unordered_set>
#include <string>

static float fade = 1;

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

static bool GameOver() {
	StopSound(SND_MUSIC);
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText("You Won!", (800 - MeasureText("You Won!", 60)) / 2, 100, 60, WHITE);
		//DrawKeybindBar("", "");
		EndDrawing();
	}
	return false;
}

const char *maps[] = {
	"44a  B"
	"||    "
	"||    "
	"||A  b"
	"Straight Across",

	"84a       "
	"||  B *   "
	"||     A  "
	"||       b"
	"Flipped Flags",

	"98a       B"
	"||         "
	"||      ***"
	"||A*       "
	"||   ***   "
	"||   ***   "
	"||   ***   "
	"||   ***  b"
	"Into the Ceiling",

	"98a        "
	"||      .  "
	"||  _      "
	"|| ******* "
	"|| *B*   & "
	"|| *A    & "
	"|| ******* "
	"||        b"
	"00" // doors
	"Button & Box",

	"97a  &  v  " // d0
	"|| _ &  _. " // b0 d1 b1
	"||   &.b   " // d2
	"||*&**    ^" // d3
	"||A   *&***" // d4
	"|| _.      " // b2
	"||        B"
	"11102" // doors
	"Three Doors"
};

enum Tile {
	T_AIR,
	T_SOLID,
	T_GOALA,
	T_GOALB,
	T_SOLIDBOTTOM,
	T_SOLIDTOP,
	T_FIRE
};

struct Box {
	int x;
	int y;
	float rendX;
	float rendY;
	int id;
};

struct Button {
	int x;
	int y;
};

struct Door {
	int x;
	int y;
	int bRef;
};

enum TurnType {
	TRN_LABEL,
	TRN_BOX,
	TRN_PLAYER
};

struct Turn {
	int type;
	int fX;
	int tX;
	int fY;
	int tY;
	int id;
};

struct Map {
	int M = 0; // moves;
	int w;
	int h;
	const char *n;
	Tile *m = nullptr; // map data
	std::vector<Box> b; // boxes
	std::vector<Button> B; // buttons
	std::vector<Door> d; // doors
	std::vector<Turn> t; // turns
};

struct Player {
	int x;
	int y;
	float rendX;
	float rendY;
	bool w; // won
	float scale = 1;
};

struct Textures {
	Texture2D bg;
	Texture2D box;
	Texture2D closed;
	Texture2D open;
	Texture2D death;
	Texture2D hole;
	Texture2D p1;
	Texture2D p2;
	Texture2D wall;
	Texture2D wallb;
	Texture2D wallt;
	Texture2D p1f;
	Texture2D p2f;

	void Load() {
		bg = LoadTexture("Background.png");
		box = LoadTexture("Box.png");
		closed = LoadTexture("Closed_Gate.png");
		death = LoadTexture("Death.png");
		hole = LoadTexture("hole.png");
		open = LoadTexture("OpenGate.png");
		p1 = LoadTexture("P1.png");
		p2 = LoadTexture("P2.png");
		wall = LoadTexture("Wall.png");
		wallb = LoadTexture("Wall_bottom_only.png");
		wallt = LoadTexture("Wall_Top_Only.png");
		p1f = LoadTexture("RedFlag.png");
		p2f = LoadTexture("BlueFlag.png");
	}

	void Unload() {
		UnloadTexture(bg);
		UnloadTexture(box);
		UnloadTexture(closed);
		UnloadTexture(death);
		UnloadTexture(hole);
		UnloadTexture(open);
		UnloadTexture(p1);
		UnloadTexture(p2);
		UnloadTexture(wall);
		UnloadTexture(wallb);
		UnloadTexture(wallt);
		UnloadTexture(p1f);
		UnloadTexture(p2f);
	}
};

struct State {
	int M = -1; // map index
	int tM = 0; // total moves
	Map m;
	Player a;
	Player b;
	bool preventMoving;
	Textures t;
	flux::Group g;
} s;

void LoadMap(const char *m /* map to load */) {
	if (s.m.m)
		delete s.m.m;
	s.m.b = {};
	s.m.B = {};
	s.m.d = {};
	s.m.t = {};
	s.m.w = (*m++) - '0';
	s.m.h = (*m++) - '0';
	s.m.m = new Tile[s.m.w * s.m.h];
	s.a = {};
	s.b = {};
	s.m.M = 0;
	fade = 1;
	s.g = flux::group();
	s.g.to(0.4f)->with(&fade, 0);
	int idx = 0;
	while (idx < s.m.w * s.m.h && *m) {
		int x = idx % s.m.w;
		int y = idx / s.m.w;
		switch (*m) {
		case '|':
			idx--;
			break;
		case 'a':
			s.a.x = x;
			s.a.y = y;
			s.a.rendX = x;
			s.a.rendY = y;
			s.m.m[idx] = T_AIR;
			break;
		case 'b':
			s.b.x = x;
			s.b.y = y;
			s.b.rendX = x;
			s.b.rendY = y;
			s.m.m[idx] = T_AIR;
			break;
		case ' ':
			s.m.m[idx] = T_AIR;
			break;
		case 'A':
			s.m.m[idx] = T_GOALA;
			break;
		case 'B':
			s.m.m[idx] = T_GOALB;
			break;
		case '*':
			s.m.m[idx] = T_SOLID;
			break;
		case '.':
			s.m.m[idx] = T_AIR;
			s.m.b.push_back(Box{ .x = x, .y = y, .rendX = (float)x, .rendY = (float)y, .id = (int)s.m.b.size() });
			break;
		case '_':
			s.m.m[idx] = T_AIR;
			s.m.B.push_back(Button{ .x = x, .y = y });
			break;
		case '&':
			s.m.m[idx] = T_AIR;
			s.m.d.push_back(Door{ .x = x, .y = y, .bRef = -1 });
			break;
		case 'v':
			s.m.m[idx] = T_SOLIDBOTTOM;
			break;
		case '^':
			s.m.m[idx] = T_SOLIDTOP;
			break;
		case '+':
			idx -= 2;
			break;
		case '!':
			s.m.m[idx] = T_FIRE;
			break;
		default:
			throw;
			break;
		}
		m++;
		idx++;
	}

	for (Door &d : s.m.d) {
		d.bRef = (*m++) - '0';
	}

	s.m.n = m;
}

bool /* game over */ LoadNextMap() {
	PlaySound(SND_WIN);
	SetSoundVolume(GetSound(SND_WIN), 2);
	if (++s.M >= sizeof(maps) / sizeof(maps[0])) {
		return true;
	}
	LoadMap(maps[s.M]);
	return false;
}

void ReloadMap() {
	s.tM -= s.m.M;
	LoadMap(maps[s.M]);
}

bool DoorOpen(const Door &d) {
	const Button &B = s.m.B[d.bRef];
	for (const Box &b : s.m.b) {
		if (b.x == B.x && b.y == B.y)
			return true;
	}
	return false;
}

void Undo() {
	if (s.m.t.empty())
		return;

	Turn t = s.m.t.back();
	s.m.t.pop_back();
	s.tM--;
	s.m.M--;

	for (int i = 0; i < t.id; i++) {
		Turn T = s.m.t.back();
		s.m.t.pop_back();

		switch (T.type) {
		case TRN_LABEL:
			break;
		case TRN_BOX:
			s.m.b[T.id].x = T.fX;
			s.m.b[T.id].y = T.fY;
			break;
		case TRN_PLAYER:
			if (T.id == 0) {
				s.a.x = T.fX;
				s.a.y = T.fY;
			}
			else {
				s.b.x = T.fX;
				s.b.y = T.fY;
			}
		}
	}
}

template <class T>
bool /* success */ TryMove(T &m, Player &o, int x, int y) {
	//if (m.w)
	//	return false;

	if (m.x + x < 0 || m.y + y < 0)
		return false;
	if (m.x + x >= s.m.w || m.y + y >= s.m.h)
		return false;

	if (m.x + x == o.x && m.y + y == o.y)
		return false;

	Tile t = s.m.m[(m.y + y) * s.m.w + m.x + x];
	Tile mt = s.m.m[m.y * s.m.w + m.x];
	
	if (t == T_SOLID)
		return false;

	if (y < 0 && t == T_SOLIDBOTTOM)
		return false;
	if (y > 0 && mt == T_SOLIDBOTTOM)
		return false;
	if (y > 0 && t == T_SOLIDTOP)
		return false;
	if (y < 0 && mt == T_SOLIDTOP)
		return false;

	for (Door &d : s.m.d)
		if (d.x == m.x + x && d.y == m.y + y && !DoorOpen(d))
			return false;

	for (Box &b : s.m.b) {
		if (b.x == m.x + x && b.y == m.y + y) {
			if (TryMove<Box>(b, o, x, y)) {
				Turn t;
				t.type = TRN_BOX;
				t.id = b.id;
				t.fX = m.x + x;
				t.fY = m.y + y;
				t.tX = b.x;
				t.tY = b.y;
				s.m.t.push_back(t);
			} else {
				return false;
			}
		}
	}

	m.x += x;
	m.y += y;

	return true;
}

bool /* success */ TryMoveA(int x, int y) {
	int fX = s.a.x;
	int fY = s.a.y;
	if (TryMove<Player>(s.a, s.b, x, y)) {
		Turn t;
		t.type = TRN_PLAYER;
		t.fX = fX;
		t.fY = fY;
		t.tX = s.a.x;
		t.tY = s.a.y;
		t.id = 0;
		s.m.t.push_back(t);
		return true;
	}
	return false;
}

bool /* success */ TryMoveB(int x, int y) {
	int fX = s.b.x;
	int fY = s.b.y;
	if (TryMove<Player>(s.b, s.a, x, y)) {
		Turn t;
		t.type = TRN_PLAYER;
		t.fX = fX;
		t.fY = fY;
		t.tX = s.b.x;
		t.tY = s.b.y;
		t.id = 1;
		s.m.t.push_back(t);
		return true;
	}
	return false;
}

bool AOverlaps(Tile t) {
	return s.m.m[s.a.y * s.m.w + s.a.x] == t;
}

bool BOverlaps(Tile t) {
	return s.m.m[s.b.y * s.m.w + s.b.x] == t;
}

void EnactMove(bool a, bool b) {
	if (!a && !b)
		return;
	s.tM++;
	s.m.M++;
	int i = 0;
	for (const Turn &t : s.m.t) {
		i++;
		if (t.type == TRN_LABEL)
			i = 0;
	}
	Turn t;
	t.type = TRN_LABEL;
	t.id = i;
	s.m.t.push_back(t);
	PlaySound(SND_FIRE);
	SetSoundVolume(GetSound(SND_FIRE), 0.2f);
}

inline float LerpPixelRound(float from, float to, float x, float max) {
	return LerpDistRound(from, to, x, max, 1.f / 32.f);
}

void DrawPlayer(Player &p, Texture2D c, bool onFire) {
	p.rendX = LerpPixelRound(p.rendX, p.x, GetFrameTime(), 0.08);
	p.rendY = LerpPixelRound(p.rendY, p.y, GetFrameTime(), 0.08);
	float o = (16 - 16 * p.scale) / 2;
	DrawTextureEx(c, Vector2{ .x = p.rendX * 16 + o, .y = p.rendY * 16 + o }, 0, p.scale, WHITE);
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	LoadNextMap();

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());
		PlaySound(SND_MUSIC);

		if (!s.preventMoving) {
			if (IsKeyPressed(KEY_R))
				ReloadMap();
			if (IsKeyPressed(KEY_U))
				Undo();

			s.a.w = AOverlaps(T_GOALA);
			s.b.w = BOverlaps(T_GOALB);

			if (AOverlaps(T_FIRE) || BOverlaps(T_FIRE)) {
				float *scale = AOverlaps(T_FIRE) ? &s.a.scale : &s.b.scale;
				s.preventMoving = true;
				s.g.to(0.4f)
					->with(scale, 0)
					->afterallelse()
					->oncomplete([=] {
					s.preventMoving = false;
					Undo();
					*scale = 1;
						});
			}

			if (s.a.w && s.b.w) {
				if (LoadNextMap()) {
					restart = GameOver();
					goto END;
				}
			}

			if (IsKeyPressed(KEY_UP)) {
				EnactMove(TryMoveA(0, -1), TryMoveB(0, 1));
			}
			if (IsKeyPressed(KEY_DOWN)) {
				EnactMove(TryMoveA(0, 1), TryMoveB(0, -1));
			}
			if (IsKeyPressed(KEY_LEFT)) {
				EnactMove(TryMoveA(-1, 0), TryMoveB(1, 0));
			}
			if (IsKeyPressed(KEY_RIGHT)) {
				EnactMove(TryMoveA(1, 0), TryMoveB(-1, 0));
			}
		}

		BeginDrawing();

		ClearBackground(BLACK);

		Camera2D c{ 0 };
		{
			int w = s.m.w * 16;
			int h = s.m.h * 16;
			float wz = SCRWID / (w + 16);
			float hz = SCRHEI / (h + 16);
			c.zoom = floor(Min(wz, hz));
			c.target.x = w / 2;
			c.target.y = h / 2;
			c.offset.x = SCRWID / 2;
			c.offset.y = SCRHEI / 2;
			c.rotation = 0;
		}

		BeginMode2D(c);

		for (int y = -2; y < s.m.h + 2; y++) {
			for (int x = -2; x < s.m.w + 2; x++) {
				if (x >= 0 && x < s.m.w
					&& y >= 0 && y < s.m.h)
					continue;
				DrawTexture(s.t.wall, x * 16, y * 16, WHITE);
			}
		}

		for (int y = 0; y < s.m.h; y++) {
			for (int x = 0; x < s.m.w; x++) {
				int i = y * s.m.w + x;
				Tile t = s.m.m[i];
				switch (t) {
				case T_AIR:
					DrawTexture(s.t.bg, x * 16, y * 16, WHITE);
					break;
				case T_SOLID:
					DrawTexture(s.t.wall, x * 16, y * 16, WHITE);
					break;
				case T_GOALA:
					DrawTexture(s.t.bg, x * 16, y * 16, WHITE);
					DrawTexture(s.t.p1f, x * 16, y * 16, WHITE);
					break;
				case T_GOALB:
					DrawTexture(s.t.bg, x * 16, y * 16, WHITE);
					DrawTexture(s.t.p2f, x * 16, y * 16, WHITE);
					break;
				case T_SOLIDBOTTOM:
					DrawTexture(s.t.bg, x * 16, y * 16, WHITE);
					DrawTexture(s.t.wallb, x * 16, y * 16, WHITE);
					break;
				case T_SOLIDTOP:
					DrawTexture(s.t.bg, x * 16, y * 16, WHITE);
					DrawTexture(s.t.wallt, x * 16, y * 16, WHITE);
					break;
				case T_FIRE:
					DrawTexture(s.t.death, x * 16, y * 16, WHITE);
				}
			}
		}

		for (Button &B : s.m.B) {
			DrawTexture(s.t.hole, B.x * 16, B.y * 16, WHITE);
		}
		for (Box &b : s.m.b) {
			b.rendX = LerpPixelRound(b.rendX, b.x, GetFrameTime(), 0.08);
			b.rendY = LerpPixelRound(b.rendY, b.y, GetFrameTime(), 0.08);
			DrawTexture(s.t.box, b.rendX * 16, b.rendY * 16, WHITE);
		}

		DrawPlayer(s.a, s.t.p1, AOverlaps(T_FIRE));
		DrawPlayer(s.b, s.t.p2, BOverlaps(T_FIRE));

		for (Door &d : s.m.d) {
			DrawTexture(DoorOpen(d) ? s.t.open : s.t.closed, d.x * 16, d.y * 16, WHITE);
		}

		//DrawParticles();

		EndMode2D();

		/*if (AnimationPlaying(ANIM_OPEN)) {
			DrawCircle(SCRWID / 2, SCRHEI / 2, 800 * (1 - (s.at / AnimationTime())), BLACK);
		}*/
		DrawCircle(SCRWID / 2, SCRHEI / 2, 800 * fade, BLACK);

		{
			const char *t = TextFormat("%d moves this map\n%d moves in total", s.m.M, s.tM);
			DrawText(t, 7, 7, 20, BLACK);
			DrawText(t, 5, 5, 20, WHITE);

			int w = MeasureText(s.m.n, 20);
			DrawText(s.m.n, SCRWID - 3 - w, 7, 20, BLACK);
			DrawText(s.m.n, SCRWID - 5 - w, 5, 20, WHITE);
		}

		DrawKeybindBar("[Up] [Down] [Left] [Right]", "[U] Undo [R] Reset");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();

	StopSound(SND_MUSIC);
	s.t.Unload();

	return restart;
}