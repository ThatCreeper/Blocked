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


float randf() {
	return (float)GetRandomValue(0, RAND_MAX - 1) / (float)RAND_MAX;
}

struct Textures {

	void Load() {
	}

	void Unload() {
	}
};

struct Player {
	int blast = 0;
	int bone = 0;
	int goo = 0;
	float topx = 0;
	int level = 0;
	float x = 0;
	float y = 600;
	float bfx = 0;
	float bfy = 0;
	float btx = 0;
	float bty = 0;
	float bt = 0;
};

/*
Kinds:
0 - good
1 - weeping
2 - archer
3 - dasher
4 - chest
5 - statue
6 - leaderboard
7 - exit door
*/
struct Enemy {
	int kind;
	float spx;
	float x;
	float dashtime;
	bool chestopen;
	bool right = true;
};

/* 
Kinds:
0 -- opening
1 -- drop high on left  --__
2 -- drop right __--
3 -- archer chest
4 -- weeping goo weeping
5 -- weeping goo
6 - 3 archer
7 - chest dasher
8 - statue
9 - leaderboard
*/

struct LevelPart {
	int kind;

	int linka = -1; // left enter/exit
	int linkb = -1; // right enter/exit
	int linkc = -1; // top enter/exit for drops
	bool fromright = false;
	bool dropped = false;

	int incnum = 0;

	static constexpr int max_enemies = 5;
	std::optional<Enemy> enemies[max_enemies];
};

struct State {
	Textures t;
	flux::Group g;
	Camera2D c;
	Player player;
	float fadetime = 0;
	std::vector<LevelPart> parts;
	float timesincedialog = 0;
	float dialogtime = 0.8f;
	bool overworld = true;
} s;

void spawnEnemy(LevelPart *p, int kind, float x, float chance) {
	float r = randf();
	if (chance != 1.f && r > (chance + p->incnum * 0.05f))
		return;
	Enemy em;
	em.kind = kind;
	em.spx = em.x = x * 800;
	em.dashtime = 0;
	em.chestopen = false;
	for (int i = 0; i < p->max_enemies; i++) {
		std::optional<Enemy> *e = p->enemies + i;
		if (e->has_value())
			continue;
		*e = em;
		break;
	}
}

int makePart(int kind, int from, bool right) {
	LevelPart *p = s.parts.data() + from; // invalid if from == -1
	LevelPart np;
	if (kind == 1 || kind == 2)
		kind = right ? 1 : 2;
	np.kind = kind;
	if (from >= 0)
		np.incnum = p->incnum + 1;
	np.fromright = right;
	for (int i = 0; i < np.max_enemies; i++) {
		np.enemies[i].reset();
	}
	switch (kind) {
	case 0:
		np.linka = -1;
		np.linkb = -1;
		np.incnum = 0;
		spawnEnemy(&np, 7, 0.5f, 1.f);
		break;
	case 1:
	case 2:
		np.linka = -1;
		np.linkb = -1;
		np.linkc = from;
		spawnEnemy(&np, 7, right ? 0.25f : 0.75f, 1.f);
		break;
	case 3:
		np.linkb = right ? -1 : from;
		np.linka = right ? from : -1;
		spawnEnemy(&np, 2, 1.f / 4.f, 0.3f);
		spawnEnemy(&np, 4, 2.f / 4.f, 1.f);
		spawnEnemy(&np, 2, 3.f / 4.f, 0.3f);
		break;
	case 4:
		np.linkb = right ? -1 : from;
		np.linka = right ? from : -1;
		spawnEnemy(&np, 1, 1.f / 4.f, 0.3f);
		spawnEnemy(&np, 0, 2.f / 4.f, 1.f);
		spawnEnemy(&np, 1, 3.f / 4.f, 0.3f);
		break;
	case 5:
		np.linkb = right ? -1 : from;
		np.linka = right ? from : -1;
		spawnEnemy(&np, 1, 1.f / 3.f, 0.66f);
		spawnEnemy(&np, 0, 2.f / 4.f, 0.66f);
		break;
	case 6:
		np.linkb = right ? -1 : from;
		np.linka = right ? from : -1;
		spawnEnemy(&np, 2, 1.f / 4.f, 0.7f);
		spawnEnemy(&np, 2, 2.f / 4.f, 0.7f);
		spawnEnemy(&np, 2, 3.f / 4.f, 0.7f);
		break;
	case 7:
		np.linkb = right ? -1 : from;
		np.linka = right ? from : -1;
		spawnEnemy(&np, 3, right ? (3.f / 4.f) : (1.f / 4.f), 0.7f);
		spawnEnemy(&np, 4, 2.f / 4.f, 1.f);
		break;
	case 8:
		np.linkb = right ? -1 : from;
		np.linka = right ? from : -1;
		spawnEnemy(&np, 5, 0.5f, 1.f);
		break;
	case 9:
		np.linkb = right ? -2 : from;
		np.linka = right ? from : -2;
		spawnEnemy(&np, 6, 0.5f, 1.f);
		break;
	}
	int idx = s.parts.size();
	s.parts.push_back(np);
	return idx;
}

bool enemyKillable(Enemy *e) {
	return e->kind <= 3;
}

void enterUnderworld();

void killPlayer() {
	s.overworld = true;
	s.player.blast /= 2;
	s.player.bone /= 2;
	s.player.goo /= 2;
	enterUnderworld();
}

void updateEnemy(Enemy *e) {
	if (e->kind == 0) {

	}
	else if (e->kind == 1) {

	}
	else if (e->kind == 2) {

	}
	else if (e->kind == 3) {
		e->dashtime += GetFrameTime();
		if (e->dashtime > 4.f)
			e->dashtime = 0.f;
		if (e->dashtime > 2.f) {
			e->x += (e->right ? 100.f : -100.f) * GetFrameTime();
		}
		if (e->x > 800 - 20) {
			e->x = 800 - 20;
			e->right = false;
		}
		if (e->x < 20) {
			e->x = 20;
			e->right = true;
		}
		bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 20 }, 20, { e->x - 20, 600 - 40, 40, 40 });
		if (overlaps)
			killPlayer();
	}
	else if (e->kind == 4) {
		if (!e->chestopen) {
			bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 20 }, 20, { e->x - 20, 600 - 40, 40, 40 });
			if (overlaps) {
				e->chestopen = true;
				s.player.blast += 5;
			}
		}
	}
	else if (e->kind == 5) {

	}
	else if (e->kind == 6) {

	}
	else if (e->kind == 7) {
		bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 20 }, 20, { e->x - 50, 600 - 200, 100, 200 });
		if (overlaps && IsKeyPressed(KEY_UP)) {
			s.overworld = true;
		}
	}
}

void drawEnemy(Enemy *e) {
	switch (e->kind) {
	case 0:
	case 1:
	case 2:
		DrawText(TextFormat("%d", e->kind), e->x, 600 - 20, 20, GREEN);
		break;
	case 3:
		DrawCircle(e->x, 600 - 20, 20, Fade(GREEN, e->dashtime > 2.f ? 1.f : 0.5f));
		break;
	case 4:
		if (e->chestopen) {
			DrawRectangle(e->x - 20, 600 - 10, 40, 10, BROWN);
		}
		else {
			DrawRectangle(e->x - 20, 600 - 40, 40, 40, YELLOW);
		}
		break;
	case 5:
		DrawRectangle(e->x - 60, 600 - 400, 120, 400, DARKPURPLE);
		break;
	case 6:
		DrawRectangle(e->x - 150, 600 - 500, 300, 500, DARKGRAY);
		break;
	case 7:
		DrawRectangle(e->x - 50, 600 - 200, 100, 200, ORANGE);
		break;
	}
}

void drawPart() {
	LevelPart *p = &s.parts[s.player.level];
	DrawText(TextFormat("#%d=%d", s.player.level, p->kind), 0, 0, 20, WHITE);

	if (p->kind == 1) {
		DrawLine(0, 300, 400, 300, BLUE);
	}
	else if (p->kind == 2) {
		DrawLine(400, 300, 800, 300, BLUE);
	}
}

void resetLevel() {
	LevelPart *p = &s.parts[s.player.level];

	for (int i = 0; i < LevelPart::max_enemies; i++) {
		std::optional<Enemy> &e = p->enemies[i];
		if (!e.has_value())
			continue;
		e->x = e->spx;
	}

	if ((p->kind == 1 || p->kind == 2) && !p->dropped)
		s.player.y = 300;
	else
		s.player.y = 600;
	s.player.bt = 0;
}

void updatePlayer() {
	LevelPart *p = &s.parts[s.player.level];
	s.player.x += (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * 120.f * GetFrameTime();

	if (p->kind == 1) {
		if (s.player.x > 400)
			p->dropped = true;
	}
	else if (p->kind == 2) {
		if (s.player.x < 400)
			p->dropped = true;
	}

	s.player.bt -= GetFrameTime();
	// shoot
	if (s.player.blast > 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		// s.player.blast--;
		s.player.bt = 1;
		s.player.bfx = s.player.x;
		s.player.bfy = s.player.y - 20;
		s.player.btx = GetMouseX();
		s.player.bty = GetMouseY();
		float d = Dist(s.player.bfx, s.player.bfy, s.player.btx, s.player.bty);
		s.player.btx -= s.player.bfx;
		s.player.bty -= s.player.bfy;
		s.player.btx /= d;
		s.player.bty /= d;
		s.player.btx += s.player.bfx;
		s.player.bty += s.player.bfy;
	}

	if (s.player.y < 600 && p->dropped)
		s.player.y += 300 * GetFrameTime();

	if (s.player.y > 600)
		s.player.y = 600;

	if (s.player.x < 20) {
		int *link = (p->kind == 1 && !p->dropped) ? &p->linkc : &p->linka;
		if (*link == -2)
			s.player.x = 20;
		else {
			/*if (*link == -1) {
				if (p->kind == 1 || p->kind == 2) {
					if (p->linkb != -1) {
						if (s.parts[p->linkb].kind != 9)
							*link = makePart(9, s.player.level, false);
						else
							*link = makePart(GetRandomValue(1, 8), s.player.level, false);
					}
					else {
						*link = makePart(GetRandomValue(1, 8), s.player.level, false);
					}
				}
				else {
					*link = makePart((p->kind == 0) ? 9 : GetRandomValue(1, 8), s.player.level, false);
				}
			}*/
			if (*link == -1) {
				int part = makePart(p->kind == 0 ? 9 : GetRandomValue(1, 8), s.player.level, false);
				p = &s.parts[s.player.level];
				link = (p->kind == 1 && !p->dropped) ? &p->linkc : &p->linka;
				*link = part;
			}
			s.player.level = *link;
			s.player.x = 800 - 20;
			resetLevel();
		}
	}
	if (s.player.x > 800 - 20) {
		int *link = (p->kind == 2 && !p->dropped) ? &p->linkc : &p->linkb;
		if (*link == -2)
			s.player.x = 800 - 20;
		else {
			/*if (*link == -1) {
				if (p->kind == 1 || p->kind == 2) {
					if (p->linka != -1) {
						if (s.parts[p->linka].kind != 9)
							*link = makePart(9, s.player.level, true);
						else
							*link = makePart(GetRandomValue(1, 8), s.player.level, true);
					}
					else {
						*link = makePart(GetRandomValue(1, 8), s.player.level, true);
					}
				}
				else {
					*link = makePart(GetRandomValue(1, 8), s.player.level, true);
				}
			}*/
			if (*link == -1) {
				int part = makePart(GetRandomValue(1, 8), s.player.level, true);
				p = &s.parts[s.player.level];
				link = (p->kind == 1 && !p->dropped) ? &p->linkc : &p->linkb;
				*link = part;
			}
			s.player.level = *link;
			s.player.x = 20;
			resetLevel();
		}
	}
}

void drawAlignRight(const char *s, int x, int y, int fontsize, Color color) {
	int wid = MeasureText(s, fontsize);
	DrawText(s, x - wid, y, fontsize, color);
}

void drawPlayer() {
	DrawCircle(s.player.x, s.player.y - 20, 20, RED);

	if (s.player.bt > 0) {
		//float at = atan2f(s.player.bty - s.player.bfy, s.player.btx - s.player.bfx);
		DrawLine(s.player.bfx, s.player.bfy, (s.player.btx - s.player.bfx) * 1000 + s.player.bfx, (s.player.bty - s.player.bfy) * 1000 + s.player.bfy, Fade(RED, s.player.bt));
		DrawCircle(s.player.bfx + (s.player.btx - s.player.bfx) * 30, s.player.bfy + (s.player.bty - s.player.bfy) * 30, 6, Fade(RED, s.player.bt));
	}

	drawAlignRight(TextFormat("%d Blast", s.player.blast), 800 - 10, 10, 20, LIGHTGRAY);
	drawAlignRight(TextFormat("%d Bone", s.player.bone), 800 - 10, 10 + 22, 20, LIGHTGRAY);
	drawAlignRight(TextFormat("%d Goo", s.player.goo), 800 - 10, 10 + 44, 20, LIGHTGRAY);
}

void enterUnderworld() {
	s.overworld = false;
	s.parts.clear();
	s.player.x = 400;
	s.player.level = makePart(0, -1, true);
	resetLevel();
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	rng = std::mt19937_64(0x007ABCD);
	s.c = { 0 };
	s.c.zoom = 1;

	enterUnderworld();

	PlaySound(SND_START);

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		s.c.offset.x = GetRenderWidth() / 2;
		s.c.offset.y = GetRenderHeight() / 2;

		Vector2 m = GetScreenToWorld2D(GetMousePosition(), s.c);
		int mtx = m.x / 32;
		int mty = m.y / 32;

		updatePlayer();

		BeginDrawing();
		rlSetLineWidth(6);

		ClearBackground(BLACK);

		if (s.overworld) {
			BeginMode2D(s.c);
			EndMode2D();
		}
		else {
			drawPart();

			for (int i = 0; i < LevelPart::max_enemies; i++) {
				std::optional<Enemy> &e = s.parts[s.player.level].enemies[i];
				if (!e.has_value())
					continue;
				updateEnemy(&*e);
				drawEnemy(&*e);
			}

			drawPlayer();
		}

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();
	

	return restart;
}