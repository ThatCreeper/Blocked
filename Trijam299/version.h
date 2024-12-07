#pragma once

// LittleBigPlanet serialization thing.
// This will not be helpful, but I wanted it.
// So it's here now.

#define SER_DEBUG 0

#undef NDEBUG
#include <assert.h>
#include <cstdint>
#include <cstdio>

enum : int32_t {
	SR_INIT = 1,

	SR_LATEST_PLUS_ONE
};
#define SR_LATEST (SR_LATEST_PLUS_ONE - 1)

struct R {
	int32_t revision = SR_LATEST;
	int32_t counter = 0;
	bool reading = true;
	FILE *file = nullptr;
#if SER_DEBUG
	FILE *debugfile = nullptr;
#endif

	inline bool IsReading() {
		return reading;
	}
	inline bool IsWriting() {
		return !reading;
	}
};

#if SER_DEBUG
#include "version_debug.h"
#else
#include "version_norm.h"
#endif

SER_BIN(int32_t, "%d");