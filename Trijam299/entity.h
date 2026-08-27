#pragma once

#include "flux.h"
#include "raylib.h"
#include "imgui.h"

struct entity;

struct entity {
	flux::Group tw;
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

#define ENT_GUI_HEAD(name) if (!guiHeader(name)) return;
