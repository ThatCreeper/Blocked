#pragma once

#include "constants.h"
#include "Entity.h"
#include "GameAction.h"
#include "ControllerAccess.h"

class SamplePlayer : public Entity {
	ControllerAccess<GameAction> ca;
	float walkSpeed = 0.f;
public:
	bool onGround();

	SamplePlayer();
	~SamplePlayer() = default;

	void onPreStepX() override;
	void onPreStepY() override;

	void preUpdate() override;
	void fixedUpdate() override;
};