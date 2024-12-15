#include "global.h"

// WHATEVER YOU DO, DO NOT ADD CLASSES PLEASE FOR THE LOVE OF GOD. OR, IF YOU DO, THINK ABOUT IT. THINK "DO I NEED THIS". THINK THAT AND THEN SAY /NO/!

#ifdef _DEBUG
// #define FORCE_EDITION 0
#endif

static bool updated = false;

void ProcessFlags(int sel) {
	updated = sel == 0 ? false : true;
}

bool PickFlags() {
#ifdef FORCE_EDITION
	ProcessFlags(FORCE_EDITION);
	return true;
#endif

	int sel = 0;
	Color overflow = BLACK;
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_UP)) {
			sel--;
			PlaySound(SND_MENU);
			overflow = BLACK;
		} if (IsKeyPressed(KEY_DOWN)) {
			sel++;
			PlaySound(SND_MENU);
			overflow = BLACK;
		}

		if (sel > 0) {
			sel = 0;
			overflow = BLUE;
		}
		else if (sel < 0) {
			sel = 0;
			overflow = ORANGE;
		}

		if (IsKeyPressed(KEY_ENTER)) {
			ProcessFlags(sel);
			return true;
		}

		BeginDrawing();

		ClearBackground(BLACK);

		// TODO: Replace this.
		DrawText("Edition of Recovery:", 15, 15, 20, WHITE);

		DrawLine(15, 45, SCRWID - 15, 45, overflow);
		DrawLine(15, 85, SCRWID - 15, 85, overflow);
		//DrawLine(15, 165, 395, 165, overflow);

		if (sel == 0) DrawRectangle(15, 50, SCRWID - 30, 30, DARKGRAY);
		if (sel == 0) DrawRectangleLines(15, 50, SCRWID - 30, 30, WHITE);
		if (sel == 0) DrawText("Version of the game made\n\nduring the three hours\n\nof the Trijam.", 25, 250, 20, WHITE);
		DrawText("3-Hour Edition", 25, 55, 20, WHITE);

		DrawKeybindBar("[Up] [Down]", "[Enter] Select");

		EndDrawing();
	}
	return false;
}

bool TrijamRunGame();

int main() {
	/*{
		R r = RRead("save.dat");
		if (r.file) {
			SER_REV(r);
			SERIALIZE(r, globstate);
		}
		RClose(r);
	}*/

	InitWindow(SCRWID, SCRHEI, "Recovery");
	InitAudioDevice();
	LoadSounds();
	SetExitKey(0);

	SetTargetFPS(30);

	if (!PickFlags())
		goto END;

	while (TrijamRunGame());

END:
	CloseWindow();
}