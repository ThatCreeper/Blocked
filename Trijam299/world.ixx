module;

#include "entity.h"

export module world;

import std;

export struct world {
	std::list<std::unique_ptr<entity>> entities;

	world() = default;
	~world() = default;
	world(const world &w) = delete;
	world(world &&w) = delete;

	// Entity should be added with `new` and should not be deleted
	void add(entity *e) {
		e->w = this;
		entities.emplace_back(e);
	}

	void remove(entity *e) {
		std::erase_if(entities, [e](const auto &_e) { return &*_e == e; });
	}

	void clear() {
		entities.clear();
	}

	void update() {
		for (auto &e : entities) {
			e->update();
		}
	}

	void render() {
		for (auto &e : entities) {
			e->render();
		}
	}

	void broadcast(signal s, void *p) {
		for (auto &e : entities) {
			e->accept(s, p);
		}
	}
};