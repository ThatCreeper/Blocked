#pragma once

#include <functional>
#include <memory>
#include <vector>

/* Based on the lua library "flux" by rxi. 
 * https://github.com/rxi/flux
 * 
 * The surface api is as similar as possible,
 * but the underlying implementation was not
 * consulted.
 * 
 * If you want to use this for anything,
 * either contact me somehow (GitHub issues?)
 * or just like use it I don't really care.
 */

namespace flux {
	class Tween;
	enum Easing;
	class Group;

	using TweenPtr = std::shared_ptr<Tween>;

	/* Updates all tweens that live on the global Group.
	 * See also: Group::update()
	 */
	void update(float deltaTime);

	/* Any number of numerical values can be tweened simultaneously.
	 * Tweens are started by using the flux::to function.
	 * This function requires one argument: the amount of time.
	 * 
	 * In rxi's flux, it used the fact that everything in Lua is
	 * a table in order to simplify the api, but this isn't true
	 * in C++. So new functions are used instead. See Tween::with.
	 * 
	 * This simply uses the global Group. See also: Group::to()
	 */
	TweenPtr to(float duration);

	class Tween : public std::enable_shared_from_this<Tween> {
	public:
		/* Example:
		 * flux::to(4)
		 *     .with(&ball.x, 200)
		 *     .with(&ball.y, 300);
		 */
		TweenPtr with(float *toUpdate, float target);

		/* Sets the easing type used by the tween.
		 * The default easing type is EASE_QUADOUT
		 */
		TweenPtr ease(Easing type);

		/* Sets the time before the tween starts.
		 * time is in seconds.
		 */
		TweenPtr delay(float time);

		/* Calls the function specified when the delay
		 * has elapsed. (During Group::Update())
		 * Functions can be stacked and will be called
		 * in a FIFO order.
		 */
		TweenPtr onstart(std::function<void()> fn);

		/* Called every time that Group::update() is
		 * called. Function will be provided deltaTime.
		 * 
		 * See also: Tween::onstart()
		 */
		TweenPtr onupdate(std::function<void(float)> fn);

		/* Calls the function specified when the tween
		 * has finished.
		 *
		 * See also: Tween::onstart()
		 */
		TweenPtr oncomplete(std::function<void()> fn);

		/* Creates a new Tween which will play after
		 * the current tween has elapsed.
		 * Returns the new tween.
		 * 
		 * See also: Group::to()
		 */
		TweenPtr after(float duration);

		/* Stops the tween and also any tweens chained
		 * with after.
		 * 
		 * Oncomplete callbacks will not be called.
		 */
		void stop();

	private:
		Tween() = default;

		bool update_(float deltaTime); // returns "should continue"

		struct Changer_ {
			float *toChange;
			float initialValue;
			float endValue;
		};
		std::vector<Changer_> changers_ = {};
		Easing easing_ = (Easing)2; /* EASE_QUADOUT */
		float delay_ = 0;
		float duration_ = 0;
		float forcedelay_ = 0;
		float time_ = 0;
		bool ended_ = false;
		bool started_ = false;
		std::vector<std::function<void()>> startfns_ = {};
		std::vector<std::function<void(float)>> updatefns_ = {};
		std::vector<std::function<void()>> endfns_ = {};
		Group *group_;
		std::shared_ptr<Tween> parent_ = nullptr;

		friend class Group;
	};

	/* See also: https://easings.net */
	enum Easing {
		EASE_LINEAR,
		EASE_QUADIN,
		EASE_QUADOUT,
		EASE_QUADINOUT,
		EASE_CUBICIN,
		EASE_CUBICOUT,
		EASE_CUBICINOUT,
		EASE_QUARTIN,
		EASE_QUARTOUT,
		EASE_QUARTINOUT,
		EASE_QUINTIN,
		EASE_QUINTOUT,
		EASE_QUINTINOUT,
		EASE_EXPOIN,
		EASE_EXPOOUT,
		EASE_EXPOINOUT,
		EASE_SINEIN,
		EASE_SINEOUT,
		EASE_SINEINOUT,
		EASE_CIRCIN,
		EASE_CIRCOUT,
		EASE_CIRCINOUT,
		EASE_BACKIN,
		EASE_BACKOUT,
		EASE_BACKINOUT,
		EASE_ELASTICIN,
		EASE_ELASTICOUT,
		EASE_ELASTICINOUT
	};

	/* Creates a new group.
	 * Groups are NOT updated alongside the global group.
	 */
	Group group();

	class Group {
	public:
		/* See also: flux::update() */
		void update(float deltatime);

		/* See also: flux::to() */
		TweenPtr to(float duration);

		Group() = default;
		~Group() = default;
		Group(const Group &) = delete;
		Group(Group &&) = delete;
	private:
		std::vector<std::shared_ptr<Tween>> tweens_;

		friend class Tween;
	};
}