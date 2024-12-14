#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>

static std::mt19937_64 rng;

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

#define MAP_WIDTH 200
#define MAP_HEIGHT 200

class Enemy {
public:
	virtual ~Enemy() = default;
	virtual void Process() = 0;
	virtual void Draw() = 0;
	virtual float GetReverseAnnoyance() = 0; // How do words?

	float x;
	float y;

protected:
	Vector2 GetNearestTree();
	void DrawAnnoyanceMeter();
};

class TreeChopper : public Enemy {
public:
	~TreeChopper() = default;
	void Process();
	void Draw();
	float GetReverseAnnoyance();

private:
	float reverseAnnoyance_ = 1;
};

struct Textures {

	void Load() {
	}

	void Unload() {
	}
};

struct Player {
	float x;
	float y;
};

struct State {
	std::vector<std::unique_ptr<Enemy>> enemies;
	std::vector<Vector2> trees;
	Textures t;
	Player p;
	flux::Group g;
} s;

bool PlayerOverlaps(Vector2 other, float rad) {
	float d = Dist(s.p.x, s.p.y, other.x, other.y);
	return d < rad + 8;
}

void TreeChopper::Process() {
	Vector2 nextPosition = GetNearestTree();
	float distX = nextPosition.x - x;
	float distY = nextPosition.y - y;
	float mag = Dist(distX, distY);
	float speed = 0.25f;

	if (!PlayerOverlaps({ x, y }, 8)) {
		x += distX / mag * speed;
		y += distY / mag * speed;
	}
	else {
		reverseAnnoyance_ -= 0.2f * GetFrameTime();
	}
}

void TreeChopper::Draw() {
	DrawCircle(x, y, 8, PURPLE);
	DrawAnnoyanceMeter();
}

float TreeChopper::GetReverseAnnoyance() {
	return reverseAnnoyance_;
}

void DrawPlayer() {
	DrawCircle(s.p.x, s.p.y, 8, GREEN);
}

void DrawTree(Vector2 v) {
	DrawCircle(v.x, v.y, 8, BROWN);
}

void PruneDeadEnemies() {
	auto it = s.enemies.begin();
	while (it != s.enemies.end()) {
		if ((*it)->GetReverseAnnoyance() <= 0) {
			it = s.enemies.erase(it);
		}
		else {
			it++;
		}
	}
}

void MovePlayerX() {
	float x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);

	for (Vector2 tree : s.trees) {
		if (PlayerOverlaps({ tree.x - x, tree.y }, 8))
			return;
	}

	for (std::unique_ptr<Enemy> &enemy : s.enemies) {
		if (PlayerOverlaps({ enemy->x - x, enemy->y }, 8))
			return;
	}

	s.p.x += x;
}

void MovePlayerY() {
	float y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);

	for (Vector2 tree : s.trees) {
		if (PlayerOverlaps({ tree.x, tree.y - y }, 8))
			return;
	}

	for (std::unique_ptr<Enemy> &enemy : s.enemies) {
		if (PlayerOverlaps({ enemy->x, enemy->y - y }, 8))
			return;
	}

	s.p.y += y;
}

void MovePlayer() {
	MovePlayerX();
	MovePlayerY();
}

static bool GameOver() {
	StopSound(SND_MUSIC);
	const char *t = TextFormat("You made $d!");
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText(t, (800 - MeasureText(t, 60)) / 2, 100, 60, WHITE);
		//DrawKeybindBar("", "");
		EndDrawing();
	}
	return false;
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	rng = std::mt19937_64(0x007ABCD);
	s.trees.push_back({ -20, 0 });
	{
		TreeChopper *chopper = new TreeChopper;
		chopper->x = -100;
		chopper->y = 80;
		s.enemies.push_back(std::unique_ptr<Enemy>(chopper));
	}

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		MovePlayer();

		for (std::unique_ptr<Enemy> &e : s.enemies) {
			e->Process();
		}
		PruneDeadEnemies();

		//PlaySound(SND_MUSIC);

		BeginDrawing();

		ClearBackground(BLACK);

		Camera2D cam;
		cam.offset.x = SCRWID / 2;
		cam.offset.y = SCRHEI / 2;
		cam.target.x = s.p.x;
		cam.target.y = s.p.y;
		cam.zoom = 2;
		cam.rotation = 0;

		BeginMode2D(cam);

		for (std::unique_ptr<Enemy> &e : s.enemies) {
			e->Draw();
		}

		for (Vector2 t : s.trees) {
			DrawTree(t);
		}

		DrawPlayer();
		
		EndMode2D();

		DrawKeybindBar("[Up] [Down] [Left] [Right]", "");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();

	//StopSound(SND_MUSIC);

	return restart;
}

Vector2 Enemy::GetNearestTree() {
	Vector2 nearest{ 0 };
	float dist = INFINITY;
	bool first = true;
	for (Vector2 tree : s.trees) {
		if (first) {
			nearest = tree;
			first = false;
			dist = Dist(x, y, tree.x, tree.y);
		}
		else {
			float d = Dist(x, y, tree.x, tree.y);
			if (d < dist) {
				nearest = tree;
				dist = d;
			}
		}
	}
	return nearest;
}

void Enemy::DrawAnnoyanceMeter() {
	if (GetReverseAnnoyance() <= 0)
		return;

	DrawRectangle(x - 6, y - 10, 12, 2, WHITE);
	DrawRectangle(x - 6, y - 10, 12 * GetReverseAnnoyance(), 2, RED);
}
