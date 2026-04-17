#include <DxLib.h>
#include "../Common/Transform.h"
#include "ColliderCapsule.h"
ColliderCapsule::ColliderCapsule(
	TAG tag, const Transform* follow,
	const VECTOR& localPosTop, const VECTOR& localPosDown, float radius)
	:
	ColliderBase(SHAPE::CAPSULE, tag, follow),
	localPosTop_(localPosTop),
	localPosDown_(localPosDown),
	radius_(radius)
{
}
ColliderCapsule::~ColliderCapsule(void)
{
}
const VECTOR& ColliderCapsule::GetLocalPosTop(void) const
{
	return localPosTop_;
}
const VECTOR& ColliderCapsule::GetLocalPosDown(void) const
{
	return localPosDown_;
}
void ColliderCapsule::SetLocalPosTop(const VECTOR& pos)
{
	localPosTop_ = pos;
}
void ColliderCapsule::SetLocalPosDown(const VECTOR& pos)
{
	localPosDown_ = pos;
}
VECTOR ColliderCapsule::GetPosTop(void) const
{
	return GetRotPos(localPosTop_);
}
VECTOR ColliderCapsule::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}
float ColliderCapsule::GetRadius(void) const
{
	return radius_;
}
void ColliderCapsule::SetRadius(float radius)
{
	radius_ = radius;
}
float ColliderCapsule::GetHeight(void) const
{
	return localPosTop_.y;
}
VECTOR ColliderCapsule::GetCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();
	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}
void ColliderCapsule::DrawDebug(int color)
{
	//// ã‚Ì‹…‘Ì
	//VECTOR pos1 = GetPosTop();
	//DrawSphere3D(pos1, radius_, 5, color, color, false);
	//// ‰º‚Ì‹…‘Ì
	//VECTOR pos2 = GetPosDown();
	//DrawSphere3D(pos2, radius_, 5, color, color, false);
	//VECTOR dir;
	//VECTOR s;
	//VECTOR e;
	//// ‹…‘Ì‚ğŒq‚®ü(X+)
	//dir = follow_->GetRight();
	//s = VAdd(pos1, VScale(dir, radius_));
	//e = VAdd(pos2, VScale(dir, radius_));
	//DrawLine3D(s, e, color);
	//// ‹…‘Ì‚ğŒq‚®ü(X-)
	//dir = follow_->GetLeft();
	//s = VAdd(pos1, VScale(dir, radius_));
	//e = VAdd(pos2, VScale(dir, radius_));
	//DrawLine3D(s, e, color);
	//// ‹…‘Ì‚ğŒq‚®ü(Z+)
	//dir = follow_->GetForward();
	//s = VAdd(pos1, VScale(dir, radius_));
	//e = VAdd(pos2, VScale(dir, radius_));
	//DrawLine3D(s, e, color);
	//// ‹…‘Ì‚ğŒq‚®ü(Z-)
	//dir = follow_->GetBack();
	//s = VAdd(pos1, VScale(dir, radius_));
	//e = VAdd(pos2, VScale(dir, radius_));
	//DrawLine3D(s, e, color);
	//// ƒJƒvƒZƒ‹‚Ì’†S
	//DrawSphere3D(GetCenter(), 5.0f, 10, color, color, true);
}