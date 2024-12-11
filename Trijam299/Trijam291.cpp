#include "global.h"

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

	InitWindow(SCRWID, SCRHEI, "Blocked");
	InitAudioDevice();
	LoadSounds();
	SetExitKey(0);

	SetTargetFPS(30);

	while (TrijamRunGame());

END:
	CloseWindow();
}