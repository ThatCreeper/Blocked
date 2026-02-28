#pragma once

#include "flux.h"
#include "raylib.h"
#include "imgui.h"

enum signal {
	SIGNAL_DESCRIBE,
	SIGNAL_GUI
};

struct entity;
struct world;

struct entityRenderer {
	entityRenderer(entity *_e) {}
	virtual ~entityRenderer() = default;
	virtual void render(entity *_e) = 0;
};

struct entity {
	flux::Group tw;
	std::unique_ptr<entityRenderer> rend;
	world *w; // set by world.add
	bool removed = false;

	virtual ~entity() {
		if (!removed) onRemove();
	};
	entity() {}

	virtual void spawnRenderer() {}
	virtual void init() {}

	virtual void update() {
		tw.update(GetFrameTime());
	}

	virtual void accept(signal s, void *p) {}

	inline void render() {
		if (rend) rend->render(this);
	}

	virtual void onRemove() {}

	inline bool guiHeader(const char *name) {
		const char *processed = TextFormat("%s %d", name, ((int)this) & 0x1FF);
		return ImGui::TreeNode(processed);
	}
};

#define ER_E(t) t *e = (t *)_e;
#define E_SLS virtual void accept(signal s, void *p) override { switch(s) {
#define E_SL(s) case s: sig##s(p); break;
#define E_SLF(fallback) default: fallback::accept(s, p); break;
#define E_SLE }}
#define E_SIGNAL(s) void sig##s(void *ptr)
#define E_SIGP(type) type *param = (type *)ptr
#define E_SIGPAR(super, s) super::sig##s(ptr)
#define E_REND(rendererClass) virtual void spawnRenderer() override { rend = std::unique_ptr<entityRenderer>(rendererClass); }
