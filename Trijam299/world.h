#include "entity.h"
#include <list>
#include <memory>
#include <algorithm>

struct World {
	std::list<std::unique_ptr<entity>> entities;
	std::vector<entity *> render_sorted_ents_dnu;

	// Entity should be added with `new` and should not be deleted
	void add(entity *e) {
		entities.emplace_back(e);
		e->init();
	}

	void remove(entity *e) {
		e->remove();
	}

	void clear() {
		for (auto &e : entities) e->remove();
		entities.clear();
	}

	void update() {
		for (auto &e : entities) {
			if (e->removed) continue;
			e->update();
		}
		std::erase_if(entities, [](const auto &e) { return e->mRefCount <= 0 && e->removed; });
	}

	void render() {
		render_sorted_ents_dnu.clear();

		for (auto &e : entities)
		{
			render_sorted_ents_dnu.push_back(e.get());
		}

		std::stable_sort(render_sorted_ents_dnu.begin(), render_sorted_ents_dnu.end(), [](entity *l, entity *r)
			{
				return l->zLayer < r->zLayer;
			});

		for (entity *e : render_sorted_ents_dnu)
		{
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