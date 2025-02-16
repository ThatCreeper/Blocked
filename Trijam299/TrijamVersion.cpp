#include "raylib.h"

#include "external/glad.h"

#include "global.h"
#include <unordered_set>
#include <string>
#include <random>
#include <format>

static std::mt19937_64 rng;

inline float LerpPixelRound(float from, float to, float x, float max, int pixelSize = 16) {
	return LerpDistRound(from, to, x, max, 0.5f / pixelSize);
}


float randf() {
	return (float)GetRandomValue(0, RAND_MAX - 1) / (float)RAND_MAX;
}

struct Textures {
	void Load() {
	}

	void Unload() {
	}
};

constexpr int goodc = 8;

// Not in state because it persists
bool disableflashingcolors = false;

struct State {
	Textures t;
	flux::Group g;

	int beat = 0;
	float timesincebeat = 0;
	float bpm = 120;

	bool goods[goodc] = { 0 };
	bool mostrecent = false;
	bool hit = false;
	const char *text = "";
	bool checked = false;
	bool hatted = false;

	int kills = 0;
	int saves = 0;

	int freebees = 5;
} s;

void ShiftGoods() {
	bool next = GetRandomValue(0, 1) == 0;
	s.mostrecent = s.goods[0];
	for (int i = 0; i < goodc - 1; i++) {
		s.goods[i] = s.goods[i + 1];
	}
	s.goods[goodc - 1] = next;
}

float GetSPB() {
	float mpb = 1 / s.bpm;
	float spb = mpb * 60;
	return spb;
}

void TryNote() {
	if (GetRandomValue(1, 5) == 1)
		return;
	int note = GetRandomValue(0, 7);
	switch (note) {
	case 0:
		PlaySound(SND_C);
		break;
	case 1:
		PlaySound(SND_D);
		break;
	case 2:
		PlaySound(SND_E);
		break;
	case 3:
		PlaySound(SND_F);
		break;
	case 4:
		PlaySound(SND_G);
		break;
	case 5:
		PlaySound(SND_A);
		break;
	case 6:
		PlaySound(SND_B);
		break;
	case 7:
		PlaySound(SND_C2);
		break;
	}
}

bool CalcBeat() {
	float spb = GetSPB();
	s.timesincebeat += GetFrameTime();
	if (!s.checked && s.timesincebeat > 0.1f) {
		if (!s.hit && s.freebees < 0) {
			return true;
		}
		s.hit = false;
		s.checked = true;
	}
	if (!s.hatted && s.timesincebeat >= spb * 0.5) {
		s.hatted = true;
		//TryNote();
	}
	while (s.timesincebeat >= spb) {
		s.timesincebeat -= spb;
		s.beat++;
		s.freebees--;

		// On Beat

		ShiftGoods();
		s.bpm += 1;
		PlaySound(SND_HAT);
		s.checked = false;
		s.hatted = false;
		TryNote();

		spb = GetSPB();
	}
	return false;
}

float GetBeatPercentage() {
	return s.timesincebeat / GetSPB();
}

float GetBeatPercentageSqr() {
	float bp = GetBeatPercentage();
	return bp * bp * bp;
}

int GetTargetBeat() {
	return s.timesincebeat < 0.1f ? s.beat : s.beat + 1;
}

bool _IsAccurate() {
	return s.timesincebeat > GetSPB() - 0.07f || s.timesincebeat < 0.1f;
}

bool IsAccurate() {
	bool acc = _IsAccurate();

	if (!acc) {
		float percent = GetBeatPercentage();
		if (percent < 0.5f)
			s.text = "Your categorization was late!";
		if (percent > 0.5f)
			s.text = "Your categorization was early!";
	}

	return acc;
}

bool IsSavePressed() {
	return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2);
}

bool IsKillPressed() {
	return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2);
}

bool IsFlashDisablePressed() {
	return IsKeyDown(KEY_F) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT);
}

bool GetTargetGood() {
	return s.timesincebeat < 0.1f ? s.mostrecent : s.goods[0];
}

bool GameOver(const char *reasontop, const char *reasonbottom) {
	EndDrawing();

	PlaySound(SND_EXPLOSION);

	while (!WindowShouldClose()) {
		if (IsFlashDisablePressed())
			disableflashingcolors = true;

		if (IsKeyPressed(KEY_ENTER) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
			if (!disableflashingcolors) DoFadeOutAnimation();
			return true;
		}
		
		BeginDrawing();

		ClearBackground(disableflashingcolors ? WHITE : RED);
		Color main = disableflashingcolors ? BLACK : WHITE;

		DrawText(reasontop, (SCRWID - MeasureText(reasontop, 50)) / 2, (SCRHEI - 50 - 30) / 2, 50, main);
		DrawText(reasonbottom, (SCRWID - MeasureText(reasonbottom, 30)) / 2, (SCRHEI - 50 - 30) / 2 + 50, 30, main);

		const char *t = TextFormat("k - %d s - %d k+s - %d bpm - %.1f", s.kills, s.saves, s.kills + s.saves, s.bpm);
		DrawText(t, (SCRWID - MeasureText(t, 20)) / 2, SCRHEI - 20 - 20 - 20 - 5, 20, main);
		const char *r = "Enter or START to restart";
		DrawText(r, (SCRWID - MeasureText(r, 20)) / 2, SCRHEI - 20 - 20, 20, main);

		EndDrawing();
	}
	return false;
}

bool TrijamRunGame() {
	int fadein = 0;
	bool restart = false;
	s = {};
	s.t.Load();
	rng = std::mt19937_64(std::random_device()());

	for (int i = 0; i < goodc; i++)
		ShiftGoods();

	PlaySound(SND_START);

	while (!WindowShouldClose()) {
		flux::update(GetFrameTime());
		s.g.update(GetFrameTime());

		if (IsFlashDisablePressed())
			disableflashingcolors = true;

		if (CalcBeat()) {
			restart = GameOver("Sleeping on the job...", "You let someone slip by unchecked");
			goto END;
		}

		if (IsKillPressed()) {
			if (!IsAccurate()) {
				restart = GameOver("Wrong time", s.text);
				goto END;
			}
			else if (GetTargetGood()) {
				restart = GameOver("Blindly shooting", "You disposed of a paying customer");
				goto END;
			}
			else {
				s.hit = true;
				PlaySound(SND_BASS);
				s.kills++;
			}
		}

		if (IsSavePressed()) {
			if (!IsAccurate()) {
				restart = GameOver("Wrong time", s.text);
				goto END;
			}
			else if (!GetTargetGood()) {
				restart = GameOver("Being nice", "You forgot your duty to dispose");
				goto END;
			}
			else {
				s.hit = true;
				PlaySound(SND_SNARE);
				s.saves++;
			}
		}

		BeginDrawing();

		ClearBackground(disableflashingcolors ? WHITE : s.freebees >= 0 ? WHITE : s.mostrecent ? GREEN : RED);

		//DrawRectangle(0, 0, 200, 50, Fade(BLACK, 1 - GetBeatPercentage()));
		//DrawRectangle(0, 0, 200 * GetBeatPercentageSqr(), 50, BLACK);

		Color linecolor = disableflashingcolors ? BLACK : s.freebees >= 0 ? BLACK : s.mostrecent ? BLACK : WHITE;

		DrawLine(SCRWID / 3, 0, SCRWID / 3, SCRHEI, linecolor);

		DrawCircle(SCRWID / 3, SCRHEI / 2, 20, Fade(linecolor, 1 - GetBeatPercentage()));
		DrawCircle(SCRWID / 3, SCRHEI / 2, 40 - 20 * GetBeatPercentageSqr(), linecolor);

		if (s.freebees < 0)
			DrawCircle(SCRWID / 3, SCRHEI / 2, 20 + 10 * GetBeatPercentage(), Fade(s.mostrecent ? GREEN : RED, Max(0.f, 1 - GetBeatPercentage() * 2)));

		for (int i = Max(0, s.freebees); i < goodc; i++) {
			DrawCircle((SCRWID / 3) + 100 * (1 + i - GetBeatPercentage()), SCRHEI / 2 + 3, 20, linecolor);
			DrawCircle((SCRWID / 3) + 100 * (1 + i - GetBeatPercentage()), SCRHEI / 2, 20, s.goods[i] ? GREEN : RED);
		}

		if (!disableflashingcolors)
			DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:

	SaveGlobState();
	s.t.Unload();
	

	return restart;
}