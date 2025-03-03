#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>
#include <format>
#include <box2d/box2d.h>

static std::mt19937_64 rng;

inline float LerpPixelRound(float from, float to, float x, float max, int pixelSize = 16) {
	return LerpDistRound(from, to, x, max, 0.5f / pixelSize);
}


float randf() {
	return (float)GetRandomValue(0, RAND_MAX - 1) / (float)RAND_MAX;
}

b2World *physWorld;

struct Textures {
	Texture2D frozen;

	void Load() {
		frozen = LoadTexture("frozen.png");
	}

	void Unload() {
		UnloadTexture(frozen);
	}
};

#define PHYS_SCALE 0.04f

struct Object {
	b2Body *b = nullptr;
	float wid = 0;
	float hei = 0;
	bool circle = false;
	bool staticBody = false;
	bool isPlayer = false;
	bool tryAttemptJoin = false;
	b2Vec2 tryAttemptJoinWhere;
	b2Body *tryAttemptJoinWith = nullptr;
	bool tryAttemptJoinBoth = false;

private:
	Object(float x, float y, bool stat, int id) {
		staticBody = stat;

		b2BodyDef bDef;
		bDef.type = stat ? b2_staticBody : b2_dynamicBody;
		bDef.position = { x * PHYS_SCALE, y * PHYS_SCALE };
		b = physWorld->CreateBody(&bDef);
		b->GetUserData().pointer = id;
	}

public:
	Object(float x, float y, float w, float h, float bounce, float dens, float frict, bool stat, int id)
	: Object(x, y, stat, id) {
		wid = w;
		hei = h;
		circle = false;

		b2PolygonShape shape;
		shape.SetAsBox(w * PHYS_SCALE / 2, h * PHYS_SCALE / 2);
		b2FixtureDef fDef;
		fDef.shape = &shape;
		fDef.density = stat ? 0 : dens;
		fDef.friction = frict;
		fDef.restitution = bounce;
		fDef.restitutionThreshold = 2.f;
		b->CreateFixture(&fDef);
	}

	Object(float x, float y, float r, float bounce, float dens, float frict, bool stat, int id)
		: Object(x, y, stat, id) {
		wid = r;
		circle = true;

		b2CircleShape shape;
		shape.m_radius = wid * PHYS_SCALE;
		b2FixtureDef fDef;
		fDef.shape = &shape;
		fDef.density = stat ? 0 : dens;
		fDef.friction = frict;
		fDef.restitution = bounce;
		fDef.restitutionThreshold = 2.f;
		b->CreateFixture(&fDef);
	}

	float GetX() {
		return b->GetTransform().p.x / PHYS_SCALE;
	}

	float GetY() {
		return b->GetTransform().p.y / PHYS_SCALE;
	}

	float GetRotation() {
		return b->GetTransform().q.GetAngle() * RAD2DEG;
	}

	void Draw() {
		if (circle) {
			DrawCircle(GetX(), GetY(), wid, staticBody ? BLUE : PINK);
			DrawRectanglePro({ GetX(), GetY(), wid * 1.414f, wid * 1.414f }, { wid * 1.414f / 2, wid * 1.414f / 2 }, GetRotation(), PURPLE);
		}
		else
			DrawRectanglePro({ GetX(), GetY(), wid, hei }, { wid / 2, hei / 2 }, GetRotation(), staticBody ? BLUE : PINK);
	}

	bool Offscreen() {
		return GetY() > SCRHEI;
	}

	bool hasBeenOffscreen = false;

	void NonPlayerUpdate();

	int framesalive = 0;

	void Update() {
		framesalive++;

		if (tryAttemptJoin) {
			tryAttemptJoin = false;
			b2RevoluteJointDef jd;
			jd.Initialize(b, tryAttemptJoinWith, tryAttemptJoinWhere);
			jd.collideConnected = !tryAttemptJoinBoth;
			jd.enableLimit = false;
			physWorld->CreateJoint(&jd);
		}

		if (!isPlayer)
			return NonPlayerUpdate();
		float rl = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
		bool altStyle = false;
		if (rl != 0)
			if (altStyle)
				b->ApplyForceToCenter(b2Mul(b->GetTransform().q, {rl * 50, 0}), true);
			else
				b->ApplyForceToCenter({ rl * 50, 0 }, true);
		
		// drag
		b->ApplyForceToCenter({ -b->GetLinearVelocity().x * 3.f, 0 }, true);
		
		//if (IsKeyPressed(KEY_UP))
		//	b->ApplyForceToCenter({ 0, -1000 }, true);
	}

	void SetRotation(float deg) {
		b->SetTransform(b->GetTransform().p, deg * DEG2RAD);
	}
};

struct State {
	Textures t;
	flux::Group g;

	std::vector<Object> b;
	int pi;
	Object *p() {
		return &b[pi];
	}

	int count = 20;
	float crot = 0;
	float toff = 0;
	bool frozen = false;

	int heavy = 0;
	bool blong = false;
	bool wide = false;
	int v = 0;
} s;

class ContactListener : public b2ContactListener {
	void AttemptJoin(b2Body *stat, b2Body *dyn, b2Contact *contact, bool both) {
		Object *o = &s.b[dyn->GetUserData().pointer];
		if (both) {
			if (s.b[dyn->GetUserData().pointer].isPlayer || s.b[stat->GetUserData().pointer].isPlayer)
				return;
		}
		if (o->framesalive < 2) {
			b2WorldManifold mani;
			contact->GetWorldManifold(&mani);
			o->tryAttemptJoin = true;
			o->tryAttemptJoinWith = stat;
			o->tryAttemptJoinWhere = mani.points[0];
			o->tryAttemptJoinBoth = both;
		}
	}

	void BeginContact(b2Contact *contact) {
		bool aStatic = contact->GetFixtureA()->GetBody()->GetType() == b2_staticBody;
		bool bStatic = contact->GetFixtureB()->GetBody()->GetType() == b2_staticBody;
		if (aStatic && !bStatic)
			AttemptJoin(contact->GetFixtureA()->GetBody(), contact->GetFixtureB()->GetBody(), contact, false);
		else if (!aStatic && bStatic)
			AttemptJoin(contact->GetFixtureB()->GetBody(), contact->GetFixtureA()->GetBody(), contact, false);
		else if (!aStatic && !bStatic) {
			AttemptJoin(contact->GetFixtureA()->GetBody(), contact->GetFixtureB()->GetBody(), contact, true);
			AttemptJoin(contact->GetFixtureB()->GetBody(), contact->GetFixtureA()->GetBody(), contact, true);
		}
	}

	void EndContact(b2Contact *contact) {

	}
};

void ClearWorld() {
	static ContactListener *l = nullptr;

	s.b.clear();
	if (physWorld) {
		delete l;
		delete physWorld;
	}
	physWorld = new b2World({ 0, 25 });
	physWorld->SetContactListener(l = new ContactListener);
}

void LoadDemoLevel() {
	s.heavy = GetRandomValue(0, 2);
	s.blong = GetRandomValue(0, 1) == 0;
	s.wide = GetRandomValue(0, 1) == 0;

	s.b.push_back(Object(150, 50, 16, 0.0f, 1.5f, 0.2f, false, s.b.size()));
	s.pi = s.b.size() - 1;
	s.p()->isPlayer = true;
	s.b.push_back(Object(150, 400 + 800 / 2, 150, 800, 0.0f, 0.0f, 0.2f, true, s.b.size()));
	s.b.push_back(Object(800 - 150, 400 + 800 / 2, 150, 800, 0.0f, 0.0f, 0.2f, true, s.b.size()));
	s.count = 20;
}

void DrawFreezeOverlay() {
	for (int i = 0; i < SCRWID; i += 30) {
		DrawTexture(s.t.frozen, i, 0, WHITE);
	}
	const char *t = "Frozen";
	int w = MeasureText(t, 30);
	DrawText(t, SCRWID - 20 - w, 20, 30, WHITE);
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	ClearWorld();
	s = {};
	s.t.Load();
	rng = std::mt19937_64(std::random_device()());

	LoadDemoLevel();

	PlaySound(SND_START);

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		s.crot -= GetMouseWheelMove() * 9;

		if (IsKeyPressed(KEY_ENTER)) {
			s.frozen = !s.frozen;
			PlaySound(s.frozen ? SND_B : SND_C);
		}

		if (s.count > 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Object o(GetMouseX(), GetMouseY(), s.blong ? 130.f : 95, s.wide ? 20 : 10, s.heavy == 2 ? 2.f : 0.0f, s.heavy == 0 ? 5.2f : 0.1f, 0.7f, false, s.b.size());
			o.SetRotation(s.crot);
			s.b.push_back(o);
			s.count--;
			s.toff = -2;
			PlaySound(SND_MENU);
		}

		//if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		//	s.b.push_back(Object(GetMouseX(), GetMouseY(), 32, 32, 0.f, 0.0f, 0.2f, true, s.b.size()));
		//}
		
		if (s.p()->GetX() > 800 - 150) {
			s.v++;
		}

		if (IsKeyReleased(KEY_R) || s.p()->Offscreen() || (s.p()->GetX() > 800 - 150)) {
			if ((s.p()->GetX() > 800 - 150)) {
				PlaySound(SND_WIN);
			}
			else {
				PlaySound(SND_EXPLOSION);
			}
			ClearWorld();
			LoadDemoLevel();
		}
		
		if (!s.frozen) {
			for (Object &o : s.b)
				o.Update();
			physWorld->Step(GetFrameTime(), 4, 4);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		for (Object &o : s.b)
			o.Draw();

		DrawRectanglePro({ (float)GetMouseX(), (float)GetMouseY(), s.blong ? 130.f : 95.f, s.wide ? 20.f : 10.f }, { (s.blong ? 130.f : 95.f) / 2, (s.wide ? 20.f : 10.f) / 2 }, s.crot, Fade(GREEN, 0.5f));

		if (s.frozen)
			DrawFreezeOverlay();

		s.toff = LerpPixelRound(s.toff, 0, GetFrameTime(), 0.1f, 1);
		{
			const char *t = TextFormat("%d", s.count);
			DrawText(t, 20, 22 + s.toff, 40, WHITE);
			DrawText(t, 20, 20 + s.toff, 40, RED);

			DrawText(TextFormat("%s %s %s", s.heavy == 0 ? "Heavy" : s.heavy == 1 ? "Light" : "Bouncy", s.blong ? "Long" : "Short", s.wide ? "Wide" : "Thin"), 20, 20 + 40 + 10, 20, WHITE);
		}

		DrawText(TextFormat("%d", s.v), 20, SCRHEI - 45 - 40, 40, WHITE);

		DrawKeybindBar("[Left/Right] [LClick] [Scroll]", "[Enter] Freeze [R] Restart");

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();
	

	return restart;
}

inline void Object::NonPlayerUpdate() {
	if (staticBody)
		return;

	if (!hasBeenOffscreen && Offscreen()) {
		s.count++;
		s.toff = 4;
		hasBeenOffscreen = true;
		PlaySound(SND_SNARE);
	}
}
