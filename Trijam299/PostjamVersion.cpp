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
	Texture2D bg;
	Texture2D player;
	Texture2D door;
	Font fnt;
	Texture2D webstatue;
	Texture2D archer;

	void Load() {
		bg = LoadTexture("bg.png");
		player = LoadTexture("player.png");
		door = LoadTexture("door.png");
		fnt = LoadFont("font.png");
		webstatue = LoadTexture("webstatue.png");
		archer = LoadTexture("archer.png");
	}

	void Unload() {
		UnloadTexture(bg);
		UnloadTexture(player);
		UnloadTexture(door);
		UnloadFont(fnt);
		UnloadTexture(webstatue);
		UnloadTexture(archer);
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
	float arrowX[3];
	float arrowY[3];
	float arrowVx[3];
	float arrowVy[3];
	float arrowtime[3];
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
	//flux::Group g;
	Camera2D c;
	Player player;
	float fadetime = 0;
	std::vector<LevelPart> parts;
	float timesincedialog = 0;
	float dialogtime = 0.8f;
	bool overworld = true;
	int webbingtondesire = 0;
	int webbingtonquant = 0;
	bool forcewebbingtonrequestdialog = false;
	int reqnum = 0;
	int dialogline = 0;
	bool dialog = false;
	float frametime = 0;
	int frame = 0;
} s;

void webbingtonNewAsk() {
	s.reqnum++;
	s.forcewebbingtonrequestdialog = true;
	s.webbingtondesire = GetRandomValue(0, 1);
	s.webbingtonquant = GetRandomValue(3, 5 * s.reqnum);
}

struct DialogLine {
	const char *t;
	bool w;
	int n;
};

DialogLine lines[] = {
	{ "My wife left me", true, 1 }, // 0
	{ "You've told me this, Webberton", false, 2 }, // 1
	{ "Well, I'm still thinking about it, Peter", true, 3 }, // 2
	{ "That's not my name!", false, -1 }, // 3
	{ "What's up with your hair?", false, 5}, // 4
	{ "What do you mean?", true, 6 }, // 5
	{ "It's funky", false, 7 }, // 6
	{ "That's rude", true, -1 }, // 7
	{ "You have issues, Webberton", false, 9 }, // 8
	{ "That's on the nose, Pettorius", true, 10 }, // 9
	{ "I don't have a nose..", false, 11 }, // 10
	{ "Unlike YOU", false, 12 }, // 11
	{ "I don't...", true, 13 }, // 12
	{ "Do I have a nose?", true, -1 } // 13
};

int startLines[] = { 0, 4, 8 };

bool hasRequest() {
	if (s.webbingtondesire == 0)
		return s.player.goo >= s.webbingtonquant;
	else
		return s.player.bone >= s.webbingtonquant;
}

void removeRequest() {
	if (s.webbingtondesire == 0)
		s.player.goo -= s.webbingtonquant;
	else
		s.player.bone -= s.webbingtonquant;
}

void playVoice() {
	if (!s.dialog)
		return;

	for (int i = SND_WEB1; i <= SND_PTR4; i++)
		StopSound((SoundID)i);

	int r = GetRandomValue(0, 3);
	bool w = s.dialogline < 0 || lines[s.dialogline].w;

	if (w) {
		switch (r) {
		case 0:
			PlaySound(SND_WEB1);
			break;
		case 1:
			PlaySound(SND_WEB2);
			break;
		case 2:
			PlaySound(SND_WEB3);
			break;
		case 3:
			PlaySound(SND_WEB4);
			break;
		}
	}
	else {
		switch (r) {
		case 0:
			PlaySound(SND_PTR1);
			break;
		case 1:
			PlaySound(SND_PTR2);
			break;
		case 2:
			PlaySound(SND_PTR3);
			break;
		case 3:
			PlaySound(SND_PTR4);
			break;
		}
	}
}

void beginDialog() {
	s.dialog = true;
	if (s.forcewebbingtonrequestdialog) {
		s.dialogline = -1;
		s.forcewebbingtonrequestdialog = false;
	}
	else if (hasRequest()) {
		removeRequest();
		s.dialogline = -2;
	}
	else {
		s.dialogline = startLines[GetRandomValue(0, (sizeof(startLines) / sizeof(*startLines)) - 1)];
	}
	playVoice();
}

void spawnEnemy(LevelPart *p, int kind, float x, float chance) {
	float r = randf();
	if (chance != 1.f && r > (chance + p->incnum * 0.05f))
		return;
	Enemy em;
	em.kind = kind;
	em.spx = em.x = x * 800;
	em.dashtime = 0;
	em.chestopen = false;
	for (int i = 0; i < 3; i++) {
		em.arrowtime[i] = 0;
		em.arrowX[i] = 0;
		em.arrowY[i] = 600;
		em.arrowVx[i] = 0;
		em.arrowVy[i] = 0;
	}
	if (kind == 2) {
		for (int i = 0; i < 3; i++) {
			em.arrowtime[i] = randf() * i + 0.3f;
		}
	}
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
	if (e->kind > 3)
		return false;
	if (e->kind == 3 && e->dashtime < 2.f)
		return false;
	return true;
}

void enterUnderworld();
void enterOverworld();

void killPlayer() {
	enterOverworld();
	s.player.blast /= 2;
	s.player.bone /= 2;
	s.player.goo /= 2;
}

void updateEnemy(Enemy *e) {
	if (e->kind == 0) {
		bool playerToRight = s.player.x > e->x;
		e->x += (playerToRight ? 60.f : -60.f) * GetFrameTime();
		bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 30 }, 30, { e->x - 20, 600 - 40, 40, 40 });
		if (overlaps)
			killPlayer();
	}
	else if (e->kind == 1) {
		float mx = GetMouseX();
		float my = GetMouseY();
		mx -= s.player.x;
		my -= s.player.y;
		mx *= 1000;
		my *= 1000;
		mx += s.player.x;
		my += s.player.y;
		bool lookingAtMe = CheckCollisionCircleLine({ e->x, 600 - 30 }, 30, { s.player.x, s.player.y }, { mx, my });
		if (!lookingAtMe) {
			e->dashtime += GetFrameTime() * 0.5f;
			bool playerToRight = s.player.x > e->x;
			e->x += (playerToRight ? 400.f : -400.f) * GetFrameTime() * Clamp(e->dashtime - 0.0f, 0.f, 1.f);
			bool overlaps = CheckCollisionCircles({ s.player.x, s.player.y - 30 }, 30, { e->x, 600 - 30 }, 20);
			if (overlaps)
				killPlayer();
		}
	}
	else if (e->kind == 2) {
		e->dashtime += GetFrameTime();
		for (int i = 0; i < 3; i++) {
			if (e->arrowY[i] > 600 && e->dashtime > e->arrowtime[i]) {
				e->arrowX[i] = e->x;
				e->arrowY[i] = 600 - 30;
				e->arrowVy[i] = -500;
				e->arrowVx[i] = (s.player.x - e->x + randf() * 50) / 2.5f;
				PlaySound(SND_ARCHER);
			}
			e->arrowVy[i] += 400 * GetFrameTime();
			e->arrowY[i] += e->arrowVy[i] * GetFrameTime();
			e->arrowX[i] += e->arrowVx[i] * GetFrameTime();

			if (CheckCollisionPointCircle({ e->arrowX[i], e->arrowY[i] }, { s.player.x, s.player.y - 30 }, 30))
				killPlayer();
		}
	}
	else if (e->kind == 3) {
		e->dashtime += GetFrameTime();
		if (e->dashtime > 4.f)
			e->dashtime = 0.f;
		if (e->dashtime > 2.f) {
			e->x += (e->right ? 400.f : -400.f) * GetFrameTime();
		}
		if (e->x > 800 - 20) {
			e->x = 800 - 20;
			e->right = false;
		}
		if (e->x < 20) {
			e->x = 20;
			e->right = true;
		}
		bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 30 }, 30, { e->x - 20, 600 - 40, 40, 40 });
		if (overlaps)
			killPlayer();
	}
	else if (e->kind == 4) {
		if (!e->chestopen) {
			bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 30 }, 30, { e->x - 20, 600 - 40, 40, 40 });
			if (overlaps) {
				e->chestopen = true;
				s.player.blast += 2;
			}
		}
	}
	else if (e->kind == 5) {

	}
	else if (e->kind == 6) {

	}
	else if (e->kind == 7) {
		bool overlaps = CheckCollisionCircleRec({ s.player.x, s.player.y - 30 }, 30, { e->x - 50, 600 - 200, 100, 200 });
		if (overlaps && IsKeyPressed(KEY_UP)) {
			enterOverworld();
		}
	}
}

void drawEnemy(Enemy *e) {
	switch (e->kind) {
	case 0:
		DrawRectangle(e->x - 30, 600 - 40, 60, 40, PINK);
		break;
	case 1:
		DrawCircle(e->x, 600 - 30, 30, { 127, 0, 0, 255 });
		break;
	case 2:
		//DrawCircleLines(e->x, 600 - 30, 30, PURPLE);
		DrawTextureRec(s.t.archer, { 60.f * (s.frame % 3), 0, 60, 60 }, { e->x - 30, 600.f - s.t.archer.height }, WHITE);
		// top eye
		{
			DrawCircle(e->x, 600 - 30 - 8, 3, PURPLE);
		}
		// bottom eye
		{
			DrawCircle(Clamp(s.player.x, e->x - 10, e->x + 10), 600 - 30 + 8, 3, PURPLE);
		}
		for (int i = 0; i < 3; i++) {
			DrawCircle(e->arrowX[i], e->arrowY[i], 2, PURPLE);
		}
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
		DrawTexture(s.t.webstatue, e->x - s.t.webstatue.width / 2, 600 - s.t.webstatue.height, WHITE);
		break;
	case 6:
		DrawRectangle(e->x - 150, 600 - 500, 300, 500, DARKGRAY);
		break;
	case 7:
		DrawTexture(s.t.door, e->x - s.t.door.width / 2, 600 - s.t.door.height, WHITE);
		break;
	}
}

void drawPart() {
	LevelPart *p = &s.parts[s.player.level];
	DrawTextEx(s.t.fnt, TextFormat("#%d=%d", s.player.level, p->kind), { 0, 0 }, 20, 2, WHITE);

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
		e->dashtime = 0;
		for (int i = 0; i < 3; i++) {
			e->arrowY[i] = 600;
			e->arrowVx[i] = 0;
			e->arrowVy[i] = 0;
		}
	}

	if ((p->kind == 1 || p->kind == 2) && !p->dropped)
		s.player.y = 300;
	else
		s.player.y = 600;
	s.player.bt = 0;
}

void updatePlayer() {
	LevelPart *p = &s.parts[s.player.level];
	s.player.x += (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * 230.f * GetFrameTime();

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
		s.player.blast--;
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

		for (int i = 0; i < p->max_enemies; i++) {
			std::optional<Enemy> &e = p->enemies[i];
			if (!e.has_value())
				continue;
			if (!enemyKillable(&*e))
				continue;
			if (CheckCollisionCircleLine({ e->x, 600 - 30 }, 30, { s.player.bfx, s.player.bfy }, { (s.player.btx - s.player.bfx) * 1000 + s.player.bfx, (s.player.bty - s.player.bfy) * 1000 + s.player.bfy })) {
				if (e->kind == 0)
					s.player.goo += 3;
				if (e->kind == 1)
					s.player.bone += 4;
				if (e->kind == 2)
					s.player.bone += 5;
				if (e->kind == 3)
					s.player.goo += 2;
				e.reset();
			}
		}
	}

	if (s.player.y < 600 && p->dropped)
		s.player.y += 300 * GetFrameTime();

	if (s.player.y > 600)
		s.player.y = 600;

	if (s.player.x < 30) {
		int *link = (p->kind == 1 && !p->dropped) ? &p->linkc : &p->linka;
		if (*link == -2)
			s.player.x = 30;
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
			s.player.x = 800 - 30;
			resetLevel();
		}
	}
	if (s.player.x > 800 - 30) {
		int *link = (p->kind == 2 && !p->dropped) ? &p->linkc : &p->linkb;
		if (*link == -2)
			s.player.x = 800 - 30;
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
			s.player.x = 30;
			resetLevel();
		}
	}
}

void drawAlignRight(const char *str, int x, int y, int fontsize, Color color) {
	float wid = MeasureTextEx(s.t.fnt, str, fontsize, fontsize / 10).x;
	DrawTextEx(s.t.fnt, str, { x - wid, (float)y }, fontsize, fontsize / 10, color);
}

void drawAlignCenter(const char *str, int x, int y, int fontsize, Color color) {
	float wid = MeasureTextEx(s.t.fnt, str, fontsize, fontsize / 10).x;
	DrawTextEx(s.t.fnt, str, { x - wid / 2, (float)y }, fontsize, fontsize / 10, color);
}

void drawPlayer() {
	//DrawCircle(s.player.x, s.player.y - 30, 30, { 127, 0, 0, 255 });
	//DrawCircleLines(s.player.x, s.player.y - 30, 30, RED);

	DrawTextureRec(s.t.player, { (s.frame % 3) * 60.f, 0, 60, 60 }, { s.player.x - 30, s.player.y - 60 }, WHITE);

	float mx = GetMouseX();
	float my = GetMouseY();

	// right eye
	{
		float x = mx - (s.player.x + 15 - 3);
		float y = my - (s.player.y - 30 - 15 + 6);

		float d = Dist(x, y);
		if (d > 8) {
			x /= d / 8;
			y /= d / 8;
		}
		x += (s.player.x + 15 - 3);
		y += (s.player.y - 30 - 15 + 6);
		DrawCircle(x, y, 3, RED);
	}

	// left eye
	{
		float x = mx - (s.player.x - 15 + 3);
		float y = my - (s.player.y - 30 - 15 + 6);

		float d = Dist(x, y);
		if (d > 8) {
			x /= d / 8;
			y /= d / 8;
		}
		x += (s.player.x - 15 + 3);
		y += (s.player.y - 30 - 15 + 6);
		DrawCircle(x, y, 3, RED);
	}

	if (s.player.bt > 0) {
		//float at = atan2f(s.player.bty - s.player.bfy, s.player.btx - s.player.bfx);
		DrawLine(s.player.bfx, s.player.bfy, (s.player.btx - s.player.bfx) * 1000 + s.player.bfx, (s.player.bty - s.player.bfy) * 1000 + s.player.bfy, Fade(RED, s.player.bt));
		DrawCircle(s.player.bfx + (s.player.btx - s.player.bfx) * 30, s.player.bfy + (s.player.bty - s.player.bfy) * 30, 6, Fade(RED, s.player.bt));
	}

	drawAlignRight(TextFormat("%d Blast", s.player.blast), 800 - 10, 10, 20, LIGHTGRAY);
	drawAlignRight(TextFormat("%d Bone", s.player.bone), 800 - 10, 10 + 22, 20, LIGHTGRAY);
	drawAlignRight(TextFormat("%d Goo", s.player.goo), 800 - 10, 10 + 44, 20, LIGHTGRAY);
}

void enterOverworld() {
	s.overworld = true;

	for (int i = 0; i < SND_COUNT; i++)
		StopSound((SoundID)i);

	beginDialog();
}

void enterUnderworld() {
	s.overworld = false;
	s.parts.clear();
	s.player.x = 400;
	s.player.level = makePart(0, -1, true);
	resetLevel();
	PlaySound(SND_UNDERWORLD);
}

void updateDialog() {
	if (!s.dialog)
		return;
	if (IsKeyPressed(KEY_ENTER)) {
		if (s.dialogline == -1)
			s.dialog = false;
		else if (s.dialogline == -2) {
			webbingtonNewAsk();
			beginDialog();
		}
		else
		{
			s.dialogline = lines[s.dialogline].n;
			s.dialog = s.dialogline != -1;
			playVoice();
		}
	}
}

bool PostjamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	rng = std::mt19937_64(0x007ABCD);
	s.c = { 0 };
	s.c.zoom = 1;

	webbingtonNewAsk();
	beginDialog();

	PlaySound(SND_START);
	BeginDrawing();
	EndDrawing();

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());

		s.c.offset.x = GetRenderWidth() / 2;
		s.c.offset.y = GetRenderHeight() / 2;

		Vector2 m = GetScreenToWorld2D(GetMousePosition(), s.c);
		int mtx = m.x / 32;
		int mty = m.y / 32;

		s.frametime += GetFrameTime();
		if (s.frametime >= 1.f / 6.f) {
			s.frametime = 0;
			s.frame++;
		}

		BeginDrawing();
		rlSetLineWidth(6);

		ClearBackground(BLACK);

		if (s.overworld) {
			updateDialog();

			if (!s.dialog && IsKeyPressed(KEY_DOWN))
				enterUnderworld();

			DrawTexture(s.t.bg, 0, 0, WHITE);
		}
		else {
			updatePlayer();
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

		DrawTextEx(s.t.fnt, TextFormat("Goal: %d %s", s.webbingtonquant, s.webbingtondesire == 0 ? "Goo" : "Bone"), { 20, 20 }, 20, 2, LIGHTGRAY);

		if (s.dialog) {
			const char *l;
			Color c;
			if (s.dialogline == -1) {
				l = TextFormat("Hey go get me %d %s", s.webbingtonquant, s.webbingtondesire == 0 ? "Goo" : "Bone");
				c = PINK;
			}
			else if (s.dialogline == -2) {
				l = "Thanks for those";
				c = PINK;
			}
			else {
				l = lines[s.dialogline].t;
				c = lines[s.dialogline].w ? PINK : ORANGE;
			}
			drawAlignCenter(l, 400, 600 - 40, 20, c);
		}

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();


	return restart;
}