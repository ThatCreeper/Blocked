#include "flux.h"

static flux::Group globalGroup{};

// This function will throw an exception if I
// personally have never used an ease function.
// Feel free to implement anything you need.
//
// Expects a value from 0 to 1.
// 
// See also: https://easings.net
static float GetEasedValue(flux::Easing easing, float value) {
	switch (easing) {
	case flux::EASE_LINEAR:
		return value;
	case flux::EASE_QUADOUT:
		return 1 - (1 - value) * (1 - value);
	}
	throw; // See comment at top of function.
}

void flux::update(float deltaTime)
{
	globalGroup.update(deltaTime);
}

flux::TweenPtr flux::to(float duration)
{
	return globalGroup.to(duration);
}

float flux::totalremainingtime()
{
	return globalGroup.totalremainingtime();
}

flux::Group flux::group()
{
	return Group();
}

flux::TweenPtr flux::Tween::with(float *toUpdate, float target)
{
	Changer_ c;
	c.toChange = toUpdate;
	c.initialValue = *toUpdate;
	c.endValue = target;
	changers_.push_back(c);

	return shared_from_this();
}

flux::TweenPtr flux::Tween::ease(Easing type)
{
	easing_ = type;
	
	return shared_from_this();
}

flux::TweenPtr flux::Tween::delay(float time)
{
	delay_ = time;

	return shared_from_this();
}

flux::TweenPtr flux::Tween::onstart(std::function<void()> fn)
{
	startfns_.push_back(fn);

	return shared_from_this();
}

flux::TweenPtr flux::Tween::onupdate(std::function<void(float)> fn)
{
	updatefns_.push_back(fn);

	return shared_from_this();
}

flux::TweenPtr flux::Tween::oncomplete(std::function<void()> fn)
{
	endfns_.push_back(fn);

	return shared_from_this();
}

flux::TweenPtr flux::Tween::after(float duration)
{
	TweenPtr other = group_->to(duration);
	other->forcedelay_ = duration_ + delay_ + forcedelay_;
	other->time_ = time_;
	other->parent_ = shared_from_this();

	return other;
}

void flux::Tween::stop()
{
	ended_ = true;
}

flux::TweenPtr flux::Tween::runningflag(int *flag)
{
	runningFlag_ = flag;

	return shared_from_this();
}

bool flux::Tween::update_(float deltaTime)
{
	if (ended_)
		return false;
	if (parent_ && parent_->ended_)
		return false;

	if (changers_.empty())
		return false;

	time_ += deltaTime;

	float time = time_ - delay_ - forcedelay_;
	time /= duration_;

	if (time < 0)
		return true;
	bool ret = true;

	if (!started_) {
		started_ = true;
		for (auto &changer : changers_)
			changer.initialValue = *changer.toChange;
		for (const auto &fn : startfns_)
			fn();
		if (runningFlag_)
			*runningFlag_ += 1;
	}

	if (time >= 1) {
		time = 1;
		ret = false;
	}

	for (const auto &changer : changers_) {
		float eased = GetEasedValue(easing_, time);
		*changer.toChange = changer.initialValue + (changer.endValue - changer.initialValue) * eased;
	}

	for (const auto &fn : updatefns_)
		fn(deltaTime);

	if (!ret) {
		for (const auto &fn : endfns_)
			fn();
		if (runningFlag_)
			*runningFlag_ -= 1;
	}

	return ret;
}

float flux::Tween::timeUntilEnd_()
{
	return duration_ + delay_ + forcedelay_ - time_;
}

void flux::Group::update(float deltatime)
{
	// Remove duplicates
	// Todo: this might not be necessary.
	// It's not too much code, though, so I'm keeping it for now.
	for (int i = tweens_.size() - 1; i >= 0; i--) {
		for (int j = 0; j < i; j++) {
			for (const Tween::Changer_ &ichanger : tweens_[i]->changers_) {
				auto &jchangers = tweens_[j]->changers_;
				auto it = jchangers.begin();
				while (it != jchangers.end()) {
					if (ichanger.toChange == it->toChange) {
						it = jchangers.erase(it);
					}
					else {
						it++;
					}
				}
			}
		}
	}

	// Actually Process
	auto it = tweens_.begin();
	
	while (it != tweens_.end()) {
		if ((*it)->update_(deltatime))
			it++;
		else
			it = tweens_.erase(it);
	}
}

flux::TweenPtr flux::Group::to(float duration)
{
	TweenPtr tween = std::shared_ptr<Tween>(new Tween);
	tween->group_ = this;
	tween->duration_ = duration;
	tweens_.push_back(tween);
	
	return tween;
}

float flux::Group::totalremainingtime()
{
	float remainingtime = 0;
	for (const auto &tween : tweens_) {
		float tweentime = tween->timeUntilEnd_();
		if (tweentime > remainingtime)
			remainingtime = tweentime;
	}
	return remainingtime;
}
