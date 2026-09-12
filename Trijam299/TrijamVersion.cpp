#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "external/glad.h"

#include "global.h"
#include "entity.h"
#include "world.h"

#define ARENA_WID 400
#define ARENA_START_X ((SCRWID - ARENA_WID) / 2)
#define ARENA_MAX_X (ARENA_START_X + ARENA_WID)

#define Z_LAYER_NORMAL 0
#define Z_LAYER_ENEMY 1
#define Z_LAYER_PLAYER 2
#define Z_LAYER_OVERLAY 3

namespace TrijamVersion
{

#include "resource_mess.h"

Textures gTex;
Shaders gShd;
flux::Group gFlux;
World gWorld;

struct Player;
struct State
{
	Player *player = nullptr;
	int score = 0;
	int isCutsceneCount = 0;
	float myHealth = 1;
	float enemyHealth = 1;
	float myHealthShake = 0;
	float enemyHealthShake = 0;
	float healthVisible = 0;

	void reset()
	{
		gTex = {};
		gShd = {};
		gFlux = {};
		gWorld = {};

		gTex.Load();
		gShd.Load();
	}

	void close()
	{
		gTex.Unload();
		gShd.Unload();
	}

	void gui() {
#define FDRAG( f ) ImGui::DragFloat( #f, &f );
#define FRANGE( f, m, M ) ImGui::SliderFloat( #f, &f, m, M );
#define UIM_I_RO(i) ImGui::Text(#i " = %d", i);
		ImGui::Begin( "State" );

		UIM_I_RO(score);
		UIM_I_RO(isCutsceneCount);
		FRANGE(myHealth, 0, 1);
		FRANGE(enemyHealth, 0, 1);
		FDRAG(myHealthShake);
		FDRAG(enemyHealthShake);
		FRANGE(healthVisible, 0, 1);

		ImGui::End();
	}
} s;

struct Sheep : entity
{
	DEFINE_ENT(Sheep, entity);

	float mX = 0;
	float mY = 0;
	float mLastX = 0;
	float mLastY = 0;
	float mVelX = 0;
	float mVelY = 300;
	bool mTethered = false;
	float mTetherX = 0;
	float mTetherY = 0;
	float mTetherDist = 0;
	int mGrabbedCount = 0;

	Sheep() : base() {
		initialize();
	}

	void update() override {
		base::update();

		if (s.isCutsceneCount > 0) return;

		mVelY += 300 * DELTA;

		if (mTethered)
		{
			mTetherDist += 64 * DELTA;
			float dist = Dist(mX, mY, mTetherX, mTetherY);
			if (dist > mTetherDist)
			{
				float oldX = mX;
				float oldY = mY;
				float targetX = mTetherX + (mX - mTetherX) / dist * mTetherDist;
				float targetY = mTetherY + (mY - mTetherY) / dist * mTetherDist;

				mVelX += (targetX - oldX) * DELTA * 300;
				mVelY += (targetY - oldY) * DELTA * 300;
			}
		}
		else
		{
			if (mVelY > 300) mVelY = 300;
		}

		mX += mVelX * DELTA;
		mY += mVelY * DELTA;

		if (!mTethered)
		{
			if (mY > SCRHEI)
			{
				PlaySound(SND_BASS);
				s.myHealthShake = 0.3;
				s.myHealth -= 0.005f;
			}
			if (mY < 0 && mVelY < 0 || mX < 0 || mX > SCRWID)
			{
				s.score += 200 * mGrabbedCount;
				s.enemyHealth -= 0.02f;
				s.enemyHealthShake = 0.3;
				PlaySound(SND_FIRE);
			}
		}
	}

	void initialize() {
		mX = GetRandomValue(ARENA_START_X, ARENA_MAX_X);
		mY = GetRandomValue(-1000, 0);
		mLastX = mX;
		mLastY = mY;
		mTethered = false;
	}

	void render() override {
		DrawCircleLines(mLastX, mLastY, 8, Fade(WHITE, 0.5f));
		DrawCircleLines(mX, mY, 8, WHITE);
	}
};

struct Bullet : entity
{
	DEFINE_ENT(Bullet, entity);

	float mX = 0;
	float mY = 0;

	Bullet() : base() {
		initialize();
	}

	void update() override {
		base::update();

		if (s.isCutsceneCount > 0) return;

		mY += 200 * DELTA;

		if (mY > SCRHEI + 32)
		{
			remove();
		}
	}

	void initialize() {
		mX = GetRandomValue(ARENA_START_X, ARENA_MAX_X);
		mY = GetRandomValue(-1000, 0);
	}

	void render() override {
		DrawCircle(mX, mY, 4, RED);
	}
};

struct Player : entity
{
	DEFINE_ENT(Player, entity);

	float mX = SCRWID / 2;
	float mY = SCRHEI / 2;
	std::vector<Sheep *> mTetheredSheep;

	Player() : base() {
		zLayer = Z_LAYER_PLAYER;
	}

	void update() override {
		base::update();

		if (s.isCutsceneCount > 0) return;

		float move_speed = DELTA * 300.0 / (1.0 + mTetheredSheep.size() * 0.1f);

		// ARROWS
		if (IsKeyDown(KEY_LEFT))
			mX -= move_speed;
		if (IsKeyDown(KEY_RIGHT))
			mX += move_speed;
		if (IsKeyDown(KEY_UP))
			mY -= move_speed;
		if (IsKeyDown(KEY_DOWN))
			mY += move_speed;

		// VEL MOUSE
		//float velX = GetMouseX() - mX;
		//float velY = GetMouseY() - mY;
		//float velD = Dist(velX, velY);
		//if (velD > FLT_EPSILON)
		//{
		//	velX = velX / velD * Clamp(velD, 0.0, move_speed);
		//	velY = velY / velD * Clamp(velD, 0.0, move_speed);
		//	mX += velX;
		//	mY += velY;
		//}

		// LERP MOUSE
		//mX = Lerp(mX, GetMouseX(), DELTA * 10, 1);
		//mY = Lerp(mY, GetMouseY(), DELTA * 10, 1);

		mY = Clamp(mY, 0, SCRHEI);
		mX = Clamp(mX, ARENA_START_X, ARENA_MAX_X);




		// Sheep Grapple
		std::erase_if(mTetheredSheep, [](Sheep *sheep)
			{
				if (sheep->removed)
				{
					sheep->FreeRef();
					return true;
				}
				return false;
			});

		if (IsKeyPressed(KEY_Z))
		{
			Sheep *sheep = getNearestSheep();
			if (sheep != nullptr)
			{
				mTetheredSheep.push_back(sheep);
				sheep->TakeRef();
				sheep->mTethered = true;
				sheep->mTetherDist = Dist(mX, mY, sheep->mX, sheep->mY);
				sheep->mGrabbedCount = 0;
				PlaySound(SND_MENU);
			}
		}

		for (Sheep *sheep : mTetheredSheep)
		{
			sheep->mTetherX = mX;
			sheep->mTetherY = mY;
			sheep->mGrabbedCount = mTetheredSheep.size();
		}

		if (IsKeyReleased(KEY_X))
		{
			for (Sheep *sheep : mTetheredSheep)
			{
				sheep->mTethered = false;
				sheep->FreeRef();
			}
			mTetheredSheep.clear();
		}
	}

	void render() override {
		// HEALTH BARS
		int shake_amnt = 10;
		int myShakeX = s.myHealthShake > 0 ? GetRandomValue(-shake_amnt, shake_amnt) * s.myHealthShake : 0;
		int myShakeY = s.myHealthShake > 0 ? GetRandomValue(-shake_amnt, shake_amnt) * s.myHealthShake : 0;
		int enemyShakeX = s.enemyHealthShake > 0 ? GetRandomValue(-shake_amnt, shake_amnt) * s.enemyHealthShake : 0;
		int enemyShakeY = s.enemyHealthShake > 0 ? GetRandomValue(-shake_amnt, shake_amnt) * s.enemyHealthShake : 0;

		if (s.healthVisible > 0)
		{
			DrawRectangle(
				Lerp(-32, ARENA_START_X - 32 - 4, s.healthVisible) + myShakeX,
				4 + myShakeY,
				32, SCRHEI - 8,
				MAGENTA);
			DrawRectangle(
				Lerp(SCRWID, ARENA_MAX_X + 32, s.healthVisible) + enemyShakeX,
				4 + enemyShakeY,
				32, SCRHEI - 8,
				MAGENTA);

			DrawRectangle(
				Lerp(-32, ARENA_START_X - 32 - 4, s.healthVisible) + myShakeX,
				4 + (SCRHEI - 8) * (1 - s.myHealth) + myShakeY,
				32, (SCRHEI - 8) * s.myHealth,
				GREEN);
			DrawRectangle(
				Lerp(SCRWID, ARENA_MAX_X + 32, s.healthVisible) + enemyShakeX,
				4 + (SCRHEI - 8) * (1 - s.enemyHealth) + enemyShakeY,
				32, (SCRHEI - 8) * s.enemyHealth,
				RED);
		}

		// TRUE PLAYER
		
		DrawCircleLines(mX, mY, 16, GREEN);
		DrawCircleLines(mX, mY, 8, GREEN);

		for (Sheep *sheep : mTetheredSheep)
		{
			DrawLine(mX, mY, sheep->mX, sheep->mY, WHITE);
		}

		Sheep *sheep = getNearestSheep();
		if (sheep != nullptr)
		{
			DrawLine(mX, mY, sheep->mX, sheep->mY, YELLOW);
		}
	}

	Sheep *getNearestSheep()
	{
		Sheep *linedSheep = nullptr;
		float dist = 200;
		gWorld.forEach<Sheep>([ & ](Sheep *sheep)
			{
				if (sheep->mTethered) return;
				if (sheep->mY < 0 || sheep->mY > SCRHEI || sheep->mX < ARENA_START_X || sheep->mX > ARENA_MAX_X)
				{
					return;
				}

				float d = Dist(mX, mY, sheep->mX, sheep->mY);
				if (d < dist)
				{
					dist = d;
					linedSheep = sheep;
				}
			});
		return linedSheep;
	}
};

struct PhaseOne : entity
{
	DEFINE_ENT(PhaseOne, entity);

	float mTimer = 0;

	void update() override
	{
		mTimer += DELTA;
		if (mTimer > 0.8f)
		{
			mTimer = 0;
			gWorld.add(new Sheep);
		}
	}
};

struct IntroCutscene : entity
{
	DEFINE_ENT(IntroCutscene, entity);

	IntroCutscene() : base()
	{
		zLayer = Z_LAYER_OVERLAY;
		s.isCutsceneCount++;
	}

	void onRemove() override {
		s.isCutsceneCount--;
		gFlux.to(2)->with(&s.healthVisible, 1)->ease(flux::EASE_BACKOUT);
		gWorld.add(new PhaseOne);
	}

	void update() override {
		if (IsKeyPressed(KEY_Z))
		{
			remove();
		}
	}

	void render() override {
		DrawRectangle(0, 0, SCRWID, SCRHEI, Fade(BLACK, 0.5));
	}
};

bool TrijamRunGame() {
	PlaySound(SND_START);
	bool restart = false;
	s.reset();

	gWorld.add(new IntroCutscene);
	gWorld.add(s.player = new Player);

	while ( !WindowShouldClose() )
	{
		// flux::update(GetFrameTime());
		gFlux.update( DELTA );

		s.myHealthShake -= DELTA;
		s.enemyHealthShake -= DELTA;
		gWorld.update();

		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( BLACK );

		// Arena
		DrawRectangleLines(ARENA_START_X, 0, ARENA_WID, SCRHEI, RED);

		gWorld.render();

		if (s.isCutsceneCount > 0)
		{
			DrawKeybindBarSide("[Z] Continue...", "");
		}
		else
		{
			DrawKeybindBarSide("[Z] Grab Sheep [X] Release [C] Fire", "[Arrows] Move");
		}

#if _DEBUG
		gTex.Gui();
		gShd.Gui();
		ImGui::Begin( "Entities" );
		gWorld.forEach<entity>( []( entity *e ) { e->trueGui(); } );
		ImGui::End();
		s.gui();
#endif

		rlImGuiEnd();
		EndDrawing();
	}

END:
	SaveGlobState();
	s.close();

	return restart;
}

}
