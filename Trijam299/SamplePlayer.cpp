#include "SamplePlayer.h"

bool SamplePlayer::onGround()
{
	return !destroyed() && vBase.dy == 0 && yr == 1 && level.hasCollision(cx, cy + 1);
}

SamplePlayer::SamplePlayer()
	: Entity(5, 5)
{
	auto start = level->data.l_Entities.all_PlayerStart[0];
	if (start != nullptr)
		setPosCase(start.cx, start.cy);

	vBase.setFricts(0.84, 0.94);

	camera->trackEntity(this, true);
	camera->clampToLevelBounds = true;

	ca = App::ME->controller.createAccess();
	ca.lockCondition = Game::isGameControllerLocked;

	auto b = new h2d::Bitmap(h2d::Tile::fromColor(GREEN, iwid, ihei), spr);
	b.tile.setCenterRatio(0.5f, 1.f);
}
