#include "ColliderSphere.h"
#include "../Common/Transform.h"

ColliderSphere::ColliderSphere(TAG tag, const Transform* follow, const VECTOR& localPos, float radius)
    :
    ColliderBase(SHAPE::SPHERE, tag, follow),
    localPos_(localPos),
    radius_(radius),
    worldPos_(VGet(0.0f, 0.0f, 0.0f))
{
}

ColliderSphere::~ColliderSphere(void)
{
}

VECTOR ColliderSphere::GetCenter(void) const
{
    // ★ Transform がある場合 → これまで通りローカル座標を回転＋平行移動
    if (follow_)
    {
        return GetRotPos(localPos_);
    }

    // ★ Transform がない場合 → worldPos_ をそのまま返す（弾用）
    return worldPos_;
}

void ColliderSphere::SetRadius(float radius)
{
    radius_ = radius;
}

void ColliderSphere::SetPosition(const VECTOR& worldPos)
{
    // ★ 弾などのためにワールド座標を直接セット
    worldPos_ = worldPos;
}

void ColliderSphere::DrawDebug(int color)
{
  /*  DrawSphere3D(GetCenter(), radius_, 10, color, color, false);*/
}
