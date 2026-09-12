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
	Player *player;
	int score = 0;

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
#define UIM_I_RO(i) ImGui::Text(#i " = %d", i);
		ImGui::Begin( "State" );

		UIM_I_RO(score);

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
				respawn();
			}
			if (mY < 0 && mVelY < 0 || mX < 0 || mX > SCRWID)
			{
				s.score += 200 * mGrabbedCount;
				PlaySound(SND_FIRE);
				respawn();
			}
		}
	}

	void respawn() {
		gWorld.add(new Sheep);
		removed = true;
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

struct Player : entity
{
	DEFINE_ENT(Player, entity);

	float mX = SCRWID / 2;
	float mY = SCRHEI / 2;
	std::vector<Sheep *> mTetheredSheep;

	void update() override {
		base::update();

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
		DrawCircleLines(mX, mY, 16, GREEN);

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

bool TrijamRunGame() {
	PlaySound(SND_START);
	bool restart = false;
	s.reset();

	gWorld.add(s.player = new Player);

	for (int i = 0; i < 10; i++)
	{
		gWorld.add(new Sheep());
	}

	while ( !WindowShouldClose() )
	{
		// flux::update(GetFrameTime());
		gFlux.update( DELTA );

		gWorld.update();

		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( BLACK );

		// Arena
		DrawRectangleLines(ARENA_START_X, 0, ARENA_WID, SCRHEI, RED);

		gWorld.render();

		DrawKeybindBar("[Z] Grab Sheep [X] Release [C] Fire", "[Arrows] Move");

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
