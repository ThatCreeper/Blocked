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
#define Z_LAYER_PLAYER 1
#define Z_LAYER_ENEMY 2
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
	bool wasGameVictory = false;

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
#define BCHECK(i) ImGui::Checkbox(#i, &i);
		ImGui::Begin( "State" );

		ImGui::DragInt("score", &score);
		UIM_I_RO(isCutsceneCount);
		FRANGE(myHealth, 0, 1);
		FRANGE(enemyHealth, 0, 1);
		FDRAG(myHealthShake);
		FDRAG(enemyHealthShake);
		FRANGE(healthVisible, 0, 1);
		BCHECK(wasGameVictory);

		ImGui::End();
	}

	void damageMe(float dmg)
	{
		myHealthShake = 0.3;
		myHealth -= dmg;
	}

	void damageEnemy(float dmg)
	{
		enemyHealthShake = 0.3;
		enemyHealth -= dmg;
	}
} s;

void DrawCutsceneCaption(const char *text, float alpha)
{
	int wid = MeasureText(text, 30);
	DrawText(text, (SCRWID - wid) / 2, SCRHEI / 2 + (SCRHEI / 2 - 30) / 2 + sinf(GetTime()) * 10, 30, Fade(WHITE, alpha));
}

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
				s.damageMe(0.005f);
				remove();
			}
			if (mY < 0 && mVelY < 0 || mX < 0 || mX > SCRWID)
			{
				s.score += 200 * mGrabbedCount;
				s.damageEnemy(0.02f);
				PlaySound(SND_FIRE);
				remove();
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
		zLayer = Z_LAYER_ENEMY;
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
		DrawCircleLines(mX, mY, 12, Fade(RED, 0.8));
		DrawCircleLines(mX, mY, 16, Fade(RED, 0.5));
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


		// bullets
		gWorld.forEach<Bullet>([ & ](Bullet *bullet)
			{
				if (Dist(mX, mY, bullet->mX, bullet->mY) < 8 + 4)
				{
					bullet->remove();
					PlaySound(SND_DIE);
					s.damageMe(0.1f);
				}
			});



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

		// SCORE METER
		{
			const char *fmt = TextFormat("SCORE %d", s.score);

			rlPushMatrix();
			rlRotatef(90, 0, 0, 1);

			DrawText(fmt, 10, -SCRWID + 10, 30, WHITE);

			rlPopMatrix();
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
struct PhaseThree : entity
{
	DEFINE_ENT(PhaseThree, entity);

	PhaseThree() : base()
	{
		zLayer = Z_LAYER_ENEMY;
		gFlux.to(1)
			->with(&s.myHealth, 1)
			->with(&s.enemyHealth, 1);
	}

	float mTimer = 0;
	float mTimer2 = 0;

	void update() override
	{
		mTimer += DELTA;
		if (mTimer > 0.6f)
		{
			mTimer = 0;
			gWorld.add(new Sheep);
		}
		mTimer2 += DELTA;
		if (mTimer2 > 0.7f)
		{
			mTimer2 = 0;
			gWorld.add(new Bullet);
		}

		s.enemyHealth -= DELTA / 20.f;

		if (s.enemyHealth <= 0)
		{
			remove();
			s.wasGameVictory = true;
		}
	}

	void render() override
	{
		DrawTexture(gTex.evil, 0, sinf(GetTime() * 0.3) * 5, WHITE);
	}
};

struct PhaseThreeCS : entity
{
	DEFINE_ENT(PhaseThreeCS, entity);

	int mIndex = 0;

	PhaseThreeCS() : base()
	{
		zLayer = Z_LAYER_OVERLAY;
		s.isCutsceneCount++;
		PlaySound(SND_DIE);
		gWorld.forEach<Sheep>([](Sheep *s) { s->remove(); });
		gWorld.forEach<Bullet>([](Bullet *s) { s->remove(); });
	}

	void onRemove() override {
		s.isCutsceneCount--;
		gWorld.add(new PhaseThree);
	}

	void update() override {
		base::update();

		if (IsKeyPressed(KEY_Z))
		{
			mIndex++;
			PlaySound(SND_MENU);

			if (mIndex > 0)
			{
				PlaySound(SND_START);
				remove();
			}
		}
	}

	void render() override {
		DrawRectangle(0, 0, SCRWID, SCRHEI, BLACK);

		switch (mIndex)
		{
		case 0:
			DrawTexture(gTex.cs2, 0, 0, WHITE);
			DrawCutsceneCaption("Fine. But what about this???.", 1);
			break;
		}
	}
};


struct PhaseTwo : entity
{
	DEFINE_ENT(PhaseTwo, entity);

	PhaseTwo() : base()
	{
		zLayer = Z_LAYER_ENEMY;
		gFlux.to(1)
			->with(&s.myHealth, 1)
			->with(&s.enemyHealth, 1);
	}

	float mTimer = 0;
	float mTimer2 = 0;

	void update() override
	{
		mTimer += DELTA;
		if (mTimer > 0.8f)
		{
			mTimer = 0;
			//gWorld.add(new Sheep);
		}
		mTimer2 += DELTA;
		if (mTimer2 > 0.6f)
		{
			mTimer2 = 0;
			gWorld.add(new Bullet);
		}

		s.enemyHealth -= DELTA / 20.f;

		if (s.enemyHealth <= 0)
		{
			remove();
			gWorld.add(new PhaseThreeCS);
		}
	}

	void render() override
	{
		DrawTexture(gTex.evil, 0, sinf(GetTime() * 0.3) * 5, WHITE);
	}
};

struct PhaseTwoCS : entity
{
	DEFINE_ENT(PhaseTwoCS, entity);

	int mIndex = 0;

	PhaseTwoCS() : base()
	{
		zLayer = Z_LAYER_OVERLAY;
		s.isCutsceneCount++;
		PlaySound(SND_DIE);
		gWorld.forEach<Sheep>([](Sheep *s) { s->remove(); });
		gWorld.forEach<Bullet>([](Bullet *s) { s->remove(); });
	}

	void onRemove() override {
		s.isCutsceneCount--;
		gWorld.add(new PhaseTwo);
	}

	void update() override {
		base::update();

		if (IsKeyPressed(KEY_Z))
		{
			mIndex++;
			PlaySound(SND_MENU);

			if (mIndex > 0)
			{
				PlaySound(SND_START);
				remove();
			}
		}
	}

	void render() override {
		DrawRectangle(0, 0, SCRWID, SCRHEI, BLACK);

		switch (mIndex)
		{
		case 0:
			DrawTexture(gTex.cs2, 0, 0, WHITE);
			DrawCutsceneCaption("Rude. But I have more.", 1);
			break;
		}
	}
};

struct PhaseOne : entity
{
	DEFINE_ENT(PhaseOne, entity);

	PhaseOne() : base()
	{
		zLayer = Z_LAYER_ENEMY;
		gFlux.to(1)
			->with(&s.myHealth, 1)
			->with(&s.enemyHealth, 1);
		gFlux.to(2)
			->with(&s.healthVisible, 1)
			->ease(flux::EASE_BACKOUT);
	}

	float mTimer = 0;
	float mTimer2 = 0;

	void update() override
	{
		mTimer += DELTA;
		if (mTimer > 0.8f)
		{
			mTimer = 0;
			gWorld.add(new Sheep);
		}
		mTimer2 += DELTA;
		if (mTimer2 > 1.9f)
		{
			mTimer2 = 0;
			//gWorld.add(new Bullet);
		}

		if (s.enemyHealth <= 0)
		{
			remove();
			gWorld.add(new PhaseTwoCS);
		}
	}

	void render() override
	{
		DrawTexture(gTex.evil, 0, sinf(GetTime() * 0.3) * 5, WHITE);
	}
};

struct MeetingCutscene : entity
{
	DEFINE_ENT(MeetingCutscene, entity);

	int mIndex = 0;

	MeetingCutscene() : base()
	{
		zLayer = Z_LAYER_OVERLAY;
		s.isCutsceneCount++;
		PlaySound(SND_DIE);
		gWorld.forEach<Sheep>([](Sheep *s) { s->remove(); });
		gWorld.forEach<Bullet>([](Bullet *s) { s->remove(); });
	}

	void onRemove() override {
		s.isCutsceneCount--;
		gWorld.add(new PhaseOne);
	}

	void update() override {
		base::update();

		if (IsKeyPressed(KEY_Z))
		{
			mIndex++;
			PlaySound(SND_MENU);

			if (mIndex > 6)
			{
				PlaySound(SND_START);
				remove();
			}
		}
	}

	void render() override {
		DrawRectangle(0, 0, SCRWID, SCRHEI, BLACK);

		switch (mIndex)
		{
		case 0:
			DrawTexture(gTex.cs1_1, 0, 0, WHITE);
			DrawCutsceneCaption("HEY!", 1);
			break;
		case 1:
			DrawTexture(gTex.cs_1_2, 0, 0, WHITE);
			DrawCutsceneCaption("You made my sheep jump off a bridge.", 1);
			break;
		case 2:
			DrawTexture(gTex.cs1_3, 0, 0, WHITE);
			DrawCutsceneCaption("Why?", 1);
			break;
		case 3:
			DrawTexture(gTex.cs1_4, 0, 0, WHITE);
			DrawCutsceneCaption("Because I am the SHEEPMANCER.", 1);
			break;
		case 4:
			DrawTexture(gTex.cs1_5, 0, 0, WHITE);
			DrawCutsceneCaption("Stealing sheep is just what I do.", 1);
			break;
		case 5:
			DrawTexture(gTex.cs1_6, 0, 0, WHITE);
			DrawCutsceneCaption("Well you should CUT IT OUT.", 1);
			break;
		case 6:
			DrawTexture(gTex.cs1_7, 0, 0, WHITE);
			DrawCutsceneCaption("No.", 1);
			break;
		}
	}
};

struct TutorialPhase : entity
{
	DEFINE_ENT(TutorialPhase, entity);

	float mTimer = 0;

	TutorialPhase() : base()
	{
		gFlux.to(1)
			->with(&s.myHealth, 1)
			->with(&s.enemyHealth, 1);
		zLayer = Z_LAYER_ENEMY;
	}

	void update() override
	{
		mTimer += DELTA;
		if (mTimer > 0.8f)
		{
			mTimer = 0;
			gWorld.add(new Sheep);
		}
		s.myHealth = 1;
		s.enemyHealth = 1;

		if (s.score >= 10000)
		{
			remove();
			gWorld.add(new MeetingCutscene);
		}
	}

	void render() override
	{
		DrawText("Tutorial!", 48, 16, 30, WHITE);
		DrawText("Hit a score of 10000", 48, 16 + 30 + 8, 20, WHITE);
	}
};

struct IntroCutscene : entity
{
	DEFINE_ENT(IntroCutscene, entity);

	float mFadeOut = 1;
	float mText = 0;
	int mDialog = 0;
	float mBridge = 0;

	IntroCutscene() : base()
	{
		zLayer = Z_LAYER_OVERLAY;
		s.isCutsceneCount++;

		tw.to(1)->with(&mText, 1);
	}

	void onRemove() override {
		s.isCutsceneCount--;
		gWorld.add(new TutorialPhase);
	}

	void update() override {
		base::update();

		if (mFadeOut == 0)
		{
			remove();
		}

		if (IsKeyPressed(KEY_Z))
		{
			if (mText < 1)
			{

			}
			else if (mDialog < 3)
			{
				mDialog++;
				PlaySound(SND_MENU);
			}
			else if (mFadeOut == 1)
			{
				tw.to(1)->with(&mFadeOut, 0);
				PlaySound(SND_START);
			}
		}

		if (mDialog >= 2)
		{
			mBridge = Clamp(mBridge + DELTA);
		}
	}

	void render() override {
		DrawRectangle(0, 0, SCRWID, SCRHEI, Fade(BLACK, mFadeOut));
		
		if (mFadeOut == 1)
		{
			DrawTexture(gTex.bridge, 0, 0, Fade(WHITE, mBridge));
			rlPushMatrix();
			rlTranslatef(SCRWID / 2, SCRHEI / 2, 0);
			rlRotatef(GetTime() * -30, 0, 0, 1);

			DrawTexture(gTex.sheep_circle, -200, -200, Fade(WHITE, mText));

			rlPopMatrix();

			DrawTexture(gTex.bridge_over, 0, 0, Fade(WHITE, mBridge));
			DrawRectangle(0, SCRHEI / 2, SCRWID, SCRHEI / 2, BLACK);

			switch (mDialog)
			{
			case 0:
				DrawCutsceneCaption("The sheep are jumping...", mText);
				break;
			case 1:
				DrawCutsceneCaption("Wait...", mText);
				break;
			case 2:
				DrawCutsceneCaption("Someone led them to the bridge.", mText);
				break;
			case 3:
				DrawCutsceneCaption("SIGHH. Time to get herding.", mText);
				break;
			}
		}
	}
};

bool GameOverScreen() {
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RED);

		const char *t1 = "Morale Destroyed!";
		const char *t2 = "Too many of your sheep died!";

		int wid1 = MeasureText(t1, 40);
		int wid2 = MeasureText(t2, 30);
		DrawText(t1, (SCRWID - wid1) / 2, SCRHEI / 2 - 40 - 4, 40, WHITE);
		DrawText(t2, (SCRWID - wid2) / 2, SCRHEI / 2 + 4, 30, WHITE);

		DrawKeybindBar("[Refresh] Play again", "");

		EndDrawing();
	}

	return false;
}

bool GameWinScreen() {
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(DARKGREEN);

		const char *t1 = "You Won!";
		const char *t2 = "You defeated the Sheepmancer! All is right...";

		int wid1 = MeasureText(t1, 40);
		int wid2 = MeasureText(t2, 30);
		DrawText(t1, (SCRWID - wid1) / 2, SCRHEI / 2 - 40 - 4, 40, WHITE);
		DrawText(t2, (SCRWID - wid2) / 2, SCRHEI / 2 + 4, 30, WHITE);

		DrawKeybindBar("[Refresh] Play again", "");

		EndDrawing();
	}

	return false;
}

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

		if (s.wasGameVictory)
		{
			return GameWinScreen();
		}
		if (s.myHealth <= 0)
		{
			return GameOverScreen();
		}


		BeginDrawing();
		rlImGuiBegin();

		ClearBackground( BLACK );

		// Arena
		//DrawRectangleLines(ARENA_START_X, 0, ARENA_WID, SCRHEI, RED);
		DrawTexture(gTex.bg2, 0, 0, WHITE);
		DrawTexture(gTex.bg1, ARENA_START_X, fmod(GetTime() * 200, SCRHEI) - SCRHEI, WHITE);
		DrawTexture(gTex.bg1, ARENA_START_X, fmod(GetTime() * 200, SCRHEI), WHITE);

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
