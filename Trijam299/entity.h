#pragma once

#include "flux.h"
#include "raylib.h"
#include "imgui.h"

struct entity;
struct world;

struct entity {
	flux::Group tw;
	world *w = nullptr; // set by world.add
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

	virtual void gui() {}

	virtual void render() {}

	virtual void onRemove() {}

	inline bool guiHeader(const char *name) {
		const char *processed = TextFormat("%s %d##%d", name, ((int)this) & 0x1FF, ((int)this));
		return ImGui::TreeNode(processed);
	}
};
