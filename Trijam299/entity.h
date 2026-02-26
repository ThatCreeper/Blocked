#pragma once

#include "flux.h"
#include "raylib.h"

enum signal {
	SIGNAL_DESCRIBE
};

struct entityRenderer;
struct world;

struct entity {
	flux::Group tw;
	std::unique_ptr<entityRenderer> rend;
	world *w; // set by world.add
	
	virtual void update() {
		tw.update(GetFrameTime());
	}

	virtual void accept(signal s, void *p) {}

	inline void render() {
		rend->render(this);
	}
};

struct entityRenderer {
	virtual void render(entity *_e) = 0;
};

#define ER_E(t) t *e = (t *)_e;
#define E_SLS virtual void accept(signal s, void *p) override { switch(s) {
#define E_SL(s) case s: sig##s(p); break;
#define E_SLF(fallback) default: fallback::accept(s, p); break;
#define E_SLE }}
#define E_SIGNAL(s, ptr) void sig##s(void *ptr)