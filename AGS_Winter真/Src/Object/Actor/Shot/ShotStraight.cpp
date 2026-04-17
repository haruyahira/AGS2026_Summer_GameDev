#include "../../Actor/ActorBase.h"
#include "ShotStraight.h"

ShotStraight::ShotStraight(TYPE type, int baseModelId) : ShotBase(type, baseModelId)
{
}

ShotStraight::~ShotStraight(void)
{
}

void ShotStraight::SetParam(void)
{
	shotPos.push_back(pos_);

	// ’e‚Ì‘å‚«‚³
	scales_ = { 0.4f, 0.4f, 0.4f };
	// ’e‚Ì‘¬“x
	speed_ = 8.0f;
	// ’e‚Ì¶‘¶”»’è
	isAlive_ = true;
	// ’e‚Ì¶‘¶ŠúŠÔ
	cntAlive_ = 300;
	// Õ“Ë”»’è—p‚Ì”¼Œa
	collisionRadius_ = 50.0f;
}