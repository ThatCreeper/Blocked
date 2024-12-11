#include "global.h"

void TrijamLoadGame();
bool TrijamUpdate();
void TrijamRender();
void TrijamUnloadGame();
bool GameOver();

int main() {
	/*{
		R r = RRead("save.dat");
		if (r.file) {
			SER_REV(r);
			SERIALIZE(r, globstate);
		}
		RClose(r);
	}*/

	InitWindow(SCRWID, SCRHEI, "Blocked");
	InitAudioDevice();
	LoadSounds();
	SetExitKey(0);

	SetTargetFPS(30);

	TrijamLoadGame();

	while (!WindowShouldClose()) {
		if (TrijamUpdate()) {
			GameOver();
			break;
		}
		TrijamRender();
	}

	TrijamUnloadGame();
	CloseWindow();
}