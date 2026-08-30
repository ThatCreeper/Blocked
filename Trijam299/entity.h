#pragma once

#include "flux.h"
#include "raylib.h"
#include "imgui.h"

struct entity;

struct entity {
	flux::Group tw;
	bool removed = false;
	int zLayer = 0;

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

	virtual bool trueGui() = 0;

	inline bool baseGuiHeader(const char *name) {
		const char *processed = TextFormat("%s %d##%d", name, ((int)(intptr_t)this) & 0x1FF, ((int)(intptr_t)this));
		return ImGui::TreeNode(processed);
	}
};

#define ENT_GUI_BEGIN( name ) if ( !guiHeader( name ) ) return
#define ENT_GUI_END() ImGui::TreePop()
#define DEFINE_ENT( name ) \
	public: \
	inline bool trueGui() override \
	{ \
		if ( !baseGuiHeader( name ) ) return; \
		this->gui(); \
		ImGui::TreePop(); \
	}
