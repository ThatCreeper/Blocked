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
	float mVelX = 0;
	float mVelY = 300;
	bool mTethered = false;
	float mTetherX = 0;
	float mTetherY = 0;
	float mTetherDist = 0;

	Sheep() : base() {
		initialize();
	}

	void update() override {
		mVelY += 300 * DELTA;
		if (mVelY > 300) mVelY = 300;

		if (mTethered)
		{

			float dist = Dist(mX, mY, mTetherX, mTetherY);
			if (dist > mTetherDist)
			{
				float oldX = mX;
				float oldY = mY;
				float targetX = mTetherX + (mX - mTetherX) / dist * mTetherDist;
				float targetY = mTetherY + (mY - mTetherY) / dist * mTetherDist;

				mVelX += (targetX - oldX);
				mVelY += (targetY - oldY);
			}
		}

		mX += mVelX * DELTA;
		mY += mVelY * DELTA;

		if (mY > SCRHEI)
		{
			respawn();
		}
	}

	void respawn() {
		gWorld.add(new Sheep);
		removed = true;
	}

	void initialize() {
		mX = GetRandomValue(ARENA_START_X, ARENA_MAX_X);
		mY = GetRandomValue(-1000, 0);
		mTethered = false;
	}

	void render() override {
		DrawCircleLines(mX, mY, 8, WHITE);
	}
};

struct Player : entity
{
	DEFINE_ENT(Player, entity);

	float mX = SCRWID / 2;
	float mY = SCRHEI / 2;
	Sheep *mTetheredSheep = nullptr;

	void update() override {
		base::update();

		float move_speed = DELTA * 300;

		if (IsKeyDown(KEY_LEFT))
			mX -= move_speed;
		if (IsKeyDown(KEY_RIGHT))
			mX += move_speed;
		if (IsKeyDown(KEY_UP))
			mY -= move_speed;
		if (IsKeyDown(KEY_DOWN))
			mY += move_speed;

		mY = Clamp(mY, 0, SCRHEI);
		mX = Clamp(mX, ARENA_START_X, ARENA_MAX_X);

		// Sheep Grapple
		if (mTetheredSheep != nullptr && mTetheredSheep->removed)
		{
			mTetheredSheep->FreeRef();
			mTetheredSheep = nullptr;
		}

		if (IsKeyPressed(KEY_Z))
		{
			mTetheredSheep = getNearestSheep();
			mTetheredSheep->TakeRef();
			mTetheredSheep->mTethered = true;
			mTetheredSheep->mTetherDist = Dist(mX, mY, mTetheredSheep->mX, mTetheredSheep->mY);
		}
		if (IsKeyDown(KEY_Z) && mTetheredSheep != nullptr)
		{
			mTetheredSheep->mTetherX = mX;
			mTetheredSheep->mTetherY = mY;
		}
		if (IsKeyReleased(KEY_Z) && mTetheredSheep != nullptr)
		{
			mTetheredSheep->mTethered = false;
			mTetheredSheep->FreeRef();
			mTetheredSheep = nullptr;
		}
	}

	void render() override {
		DrawCircleLines(mX, mY, 16, GREEN);

		if (mTetheredSheep != nullptr)
		{
			DrawLine(mX, mY, mTetheredSheep->mX, mTetheredSheep->mY, WHITE);
		}
		else
		{
			Sheep *sheep = getNearestSheep();
			DrawLine(mX, mY, sheep->mX, sheep->mY, YELLOW);
		}
	}

	Sheep *getNearestSheep()
	{
		Sheep *linedSheep = nullptr;
		float dist = INFINITY;
		gWorld.forEach<Sheep>([ & ](Sheep *sheep)
			{
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

		DrawKeybindBar("[Z] Grab/Release Sheep [X] Fire", "[Arrows] Move");

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
