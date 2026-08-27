#include "entity.h"
#include <list>
#include <memory>

struct World {
	std::list<std::unique_ptr<entity>> entities;

	// Entity should be added with `new` and should not be deleted
	void add(entity *e) {
		entities.emplace_back(e);
		e->spawnRenderer();
		e->init();
	}

	void remove(entity *e) {
		e->onRemove();
		e->removed = true;
	}

	void clear() {
		for (auto &e : entities) e->onRemove();
		entities.clear();
	}

	void update() {
		for (auto &e : entities) {
			if (e->removed) continue;
			e->update();
		}
		std::erase_if(entities, [](const auto &e) { return e->removed; });
	}

	void render() {
		for (auto &e : entities) {
			if (e->removed) continue;
			e->render();
		}
	}

	template <class Type, class Fn>
	void forEach(Fn fn) {
		for (auto &e : entities) {
			if constexpr (std::is_same_v<Type, entity>)
			{
				fn(&*e);
			}
			else {
				Type* casted = dynamic_cast<Type*>(&*e);
				if (casted) fn(casted);
			}
		}
	}
	
	template <class E>
	E *getFirstOfKind() {
		for (auto &e : entities) {
			E *casted = dynamic_cast<E *>(&*e);
			if (casted) return casted;
		}
		return nullptr;
	}
};