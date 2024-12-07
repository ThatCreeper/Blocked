#include "global.h"

void DrawKeybindBar(const char *left, const char *right, bool bg) {
	if (bg)
		DrawRectangle(0, SCRHEI - 30, SCRWID, 30, Fade(BLACK, 0.7f));
	DrawLine(0, SCRHEI - 31, SCRWID, SCRHEI - 31, WHITE); // I dislike the number "31" here, but it is correct. Sad.
	DrawText(left, 10, SCRHEI - 25, 20, WHITE);
	int rlen = MeasureText(right, 20);
	DrawText(right, SCRWID - 10 - rlen, SCRHEI - 25, 20, WHITE);
}

void DoFadeOutAnimation() {
	int top = 0;

	while ((top += 80) <= SCRWID) {
		float t = ((float)top) / (float)SCRWID;
		t *= t;
		t *= SCRWID;

		BeginDrawing();

		DrawRectangle(0, 0, t, SCRHEI, BLUE);

		EndDrawing();
	}
}

void DoFadeInAnimation(int &top) {
	if (top < SCRWID) {
		top += 60;
		float t = ((float)top) / (float)SCRWID;
		t *= t;
		t *= SCRWID;

		DrawRectangle(t, 0, SCRWID, SCRHEI, BLUE);
	}
}