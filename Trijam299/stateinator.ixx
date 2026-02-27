export module stateinator;

import std;

export template <
	typename beginfunc = void(),
	typename updatefunc = void()
>
struct stateinator {
	using optbegin = std::optional<std::function<beginfunc>>;
	using optupdate = std::optional<std::function<updatefunc>>;
	struct stateInfo {
		optbegin begin;
		optbegin end;
		optupdate update;
	};

	std::vector<stateInfo> states;
	int state = -1;
	int lastState = -1;

	void add(int index, optbegin begin, optbegin end, optupdate update) {
		if (states.size() != index) throw;
		states.emplace_back(begin, end, update);
	}

	template <typename... Args>
	void set(int newState, Args&&... args) {
		lastState = state;
		state = newState;

		if (lastState != -1 && states[lastState].end) states[lastState].end.value()(std::forward<Args>(args)...);
		if (states[state].begin) states[state].begin.value()(std::forward<Args>(args)...);
	}

	template <typename... Args>
	void update(Args&&... args) {
		if (states[state].update) states[state].update.value()(std::forward<Args>(args)...);
	}
};