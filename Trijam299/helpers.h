#pragma once

inline int Overflow(int x, int min, int max) {
	if (x < min)
		return max;
	if (x > max)
		return min;
	return x;
}

inline int Overflow(int x, int max) {
	return Overflow(x, 0, max);
}

inline float Clamp(float x, float min, float max) {
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}

inline int Max(int a, int b) {
	if (a > b)
		return a;
	return b;
}

inline int Min(int a, int b) {
	if (a < b)
		return a;
	return b;
}

inline float Min(float a, float b) {
	if (a < b)
		return a;
	return b;
}

inline int Clamp(int x, int min, int max) {
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}

inline float Clamp(float x, int min, int max) {
	return Clamp(x, (float)min, (float)max);
}

inline float Dist(float ax, float ay, float bx, float by) {
	float xd = ax - bx;
	float yd = ay - by;
	return sqrtf(xd * xd + yd * yd);
}

inline float Dist(float x, float y) {
	return sqrtf(x * x + y * y);
}

inline float Lerp(float from, float to, float x, float max) {
	float t = x / max;
	return from * (1 - t) + to * t;
}

// X from 0 to 1
inline float SInterp(float x) {
	return (1 - cosf(PI * x)) / 2;
}

inline float SInterp(float x, float max) {
	return SInterp(x / max);
}

inline float SInterp(float from, float to, float x, float max) {
	float i = SInterp(x, max);
	from *= 1 - i;
	to   *= i;
	return from + to;
}