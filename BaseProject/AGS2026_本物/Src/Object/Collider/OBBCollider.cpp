#include "OBBCollider.h"
#include <cmath>

OBBCollider::OBBCollider()
{
    center_ = VGet(0.0f, 0.0f, 0.0f);
    halfSize_ = VGet(1.0f, 1.0f, 1.0f);

    axisX_ = VGet(1.0f, 0.0f, 0.0f);
    axisY_ = VGet(0.0f, 1.0f, 0.0f);
    axisZ_ = VGet(0.0f, 0.0f, 1.0f);
}

OBBCollider::OBBCollider(
    VECTOR center,
    VECTOR halfSize,
    VECTOR axisX,
    VECTOR axisY,
    VECTOR axisZ)
{
    center_ = center;
    halfSize_ = halfSize;

    // 軸は必ず正規化する
    axisX_ = VNorm(axisX);
    axisY_ = VNorm(axisY);
    axisZ_ = VNorm(axisZ);
}


void OBBCollider::DrawDebug(unsigned int color) const
{
    VECTOR x = VScale(axisX_, halfSize_.x);
    VECTOR y = VScale(axisY_, halfSize_.y);
    VECTOR z = VScale(axisZ_, halfSize_.z);

    VECTOR p[8];

    p[0] = VAdd(center_, VAdd(VAdd(VScale(x, -1.0f), VScale(y, -1.0f)), VScale(z, -1.0f)));
    p[1] = VAdd(center_, VAdd(VAdd(VScale(x, 1.0f), VScale(y, -1.0f)), VScale(z, -1.0f)));
    p[2] = VAdd(center_, VAdd(VAdd(VScale(x, 1.0f), VScale(y, 1.0f)), VScale(z, -1.0f)));
    p[3] = VAdd(center_, VAdd(VAdd(VScale(x, -1.0f), VScale(y, 1.0f)), VScale(z, -1.0f)));

    p[4] = VAdd(center_, VAdd(VAdd(VScale(x, -1.0f), VScale(y, -1.0f)), VScale(z, 1.0f)));
    p[5] = VAdd(center_, VAdd(VAdd(VScale(x, 1.0f), VScale(y, -1.0f)), VScale(z, 1.0f)));
    p[6] = VAdd(center_, VAdd(VAdd(VScale(x, 1.0f), VScale(y, 1.0f)), VScale(z, 1.0f)));
    p[7] = VAdd(center_, VAdd(VAdd(VScale(x, -1.0f), VScale(y, 1.0f)), VScale(z, 1.0f)));

    // 奥面
    DrawLine3D(p[0], p[1], color);
    DrawLine3D(p[1], p[2], color);
    DrawLine3D(p[2], p[3], color);
    DrawLine3D(p[3], p[0], color);

    // 手前面
    DrawLine3D(p[4], p[5], color);
    DrawLine3D(p[5], p[6], color);
    DrawLine3D(p[6], p[7], color);
    DrawLine3D(p[7], p[4], color);

    // 接続線
    DrawLine3D(p[0], p[4], color);
    DrawLine3D(p[1], p[5], color);
    DrawLine3D(p[2], p[6], color);
    DrawLine3D(p[3], p[7], color);
}

bool OBBCollider::CheckCollision(VECTOR point) const
{
    VECTOR diff = VSub(point, center_);

    float localX = VDot(diff, axisX_);
    float localY = VDot(diff, axisY_);
    float localZ = VDot(diff, axisZ_);

    if (fabsf(localX) > halfSize_.x) return false;
    if (fabsf(localY) > halfSize_.y) return false;
    if (fabsf(localZ) > halfSize_.z) return false;

    return true;
}

bool OBBCollider::ResolveCollisionXZ(
    VECTOR& pos,
    float radius,
    float bottomY,
    float topY) const
{
    // まず高さチェック
    float boxBottom = center_.y - halfSize_.y;
    float boxTop = center_.y + halfSize_.y;

    if (bottomY > boxTop || topY < boxBottom)
    {
        return false;
    }

    // OBB中心からプレイヤー位置への差分
    VECTOR diff = VSub(pos, center_);

    // OBBのローカル座標へ変換
    float localX = VDot(diff, axisX_);
    float localZ = VDot(diff, axisZ_);

    // OBB上の最近点を求める
    float closestX = localX;
    float closestZ = localZ;

    if (closestX < -halfSize_.x) closestX = -halfSize_.x;
    if (closestX > halfSize_.x) closestX = halfSize_.x;

    if (closestZ < -halfSize_.z) closestZ = -halfSize_.z;
    if (closestZ > halfSize_.z) closestZ = halfSize_.z;

    float diffX = localX - closestX;
    float diffZ = localZ - closestZ;

    float distSq = diffX * diffX + diffZ * diffZ;

    // 半径より遠ければ当たっていない
    if (distSq >= radius * radius)
    {
        return false;
    }

    // 押し出し
    if (distSq > 0.0001f)
    {
        float dist = sqrtf(distSq);

        localX = closestX + (diffX / dist) * radius;
        localZ = closestZ + (diffZ / dist) * radius;
    }
    else
    {
        // 完全に中に入っている場合
        float penX = halfSize_.x - fabsf(localX);
        float penZ = halfSize_.z - fabsf(localZ);

        if (penX < penZ)
        {
            if (localX >= 0.0f)
            {
                localX = halfSize_.x + radius;
            }
            else
            {
                localX = -halfSize_.x - radius;
            }
        }
        else
        {
            if (localZ >= 0.0f)
            {
                localZ = halfSize_.z + radius;
            }
            else
            {
                localZ = -halfSize_.z - radius;
            }
        }
    }

    // ローカル座標からワールド座標へ戻す
    VECTOR newPos = center_;

    newPos = VAdd(newPos, VScale(axisX_, localX));
    newPos = VAdd(newPos, VScale(axisZ_, localZ));

    // Yは重力処理に任せたいのでX/Zだけ修正
    pos.x = newPos.x;
    pos.z = newPos.z;

    return true;
}
bool OBBCollider::ResolveCollisionTop(
    VECTOR& pos,
    float radius,
    float bottomY,
    float topY) const
{
    // 床OBBの上面・下面
    float boxBottom = center_.y - halfSize_.y;
    float boxTop = center_.y + halfSize_.y;

    // 高さが重なっていないなら判定しない
    if (bottomY > boxTop || topY < boxBottom)
    {
        return false;
    }

    // プレイヤー位置をOBBローカル座標に変換
    VECTOR diff = VSub(pos, center_);

    float localX = VDot(diff, axisX_);
    float localZ = VDot(diff, axisZ_);

    // XZ方向で床の範囲内に近い点を求める
    float closestX = localX;
    float closestZ = localZ;

    if (closestX < -halfSize_.x) closestX = -halfSize_.x;
    if (closestX > halfSize_.x) closestX = halfSize_.x;

    if (closestZ < -halfSize_.z) closestZ = -halfSize_.z;
    if (closestZ > halfSize_.z) closestZ = halfSize_.z;

    float diffX = localX - closestX;
    float diffZ = localZ - closestZ;

    float distSq = diffX * diffX + diffZ * diffZ;

    // プレイヤー半径より遠いなら床に乗っていない
    if (distSq > radius * radius)
    {
        return false;
    }

    // 床の上に押し上げる
    pos.y = boxTop;

    return true;
}

bool OBBCollider::ResolveCollisionBottom(
    VECTOR& pos,
    float radius,
    float bottomY,
    float topY) const
{
    float boxBottom = center_.y - halfSize_.y;
    float boxTop = center_.y + halfSize_.y;

    // 高さが重なっていないなら判定しない
    if (bottomY > boxTop || topY < boxBottom)
    {
        return false;
    }

    // XZ判定
    VECTOR diff = VSub(pos, center_);

    float localX = VDot(diff, axisX_);
    float localZ = VDot(diff, axisZ_);

    float closestX = localX;
    float closestZ = localZ;

    if (closestX < -halfSize_.x) closestX = -halfSize_.x;
    if (closestX > halfSize_.x) closestX = halfSize_.x;

    if (closestZ < -halfSize_.z) closestZ = -halfSize_.z;
    if (closestZ > halfSize_.z) closestZ = halfSize_.z;

    float diffX = localX - closestX;
    float diffZ = localZ - closestZ;

    float distSq = diffX * diffX + diffZ * diffZ;

    if (distSq > radius * radius)
    {
        return false;
    }

    // 頭が天井の下面に当たったら、下に押し戻す
    pos.y = boxBottom - (topY - bottomY);

    return true;
}

bool OBBCollider::ResolveSphere(
    VECTOR& pos,
    float radius) const
{
    // OBB中心からカメラ位置への差分
    VECTOR diff = VSub(pos, center_);

    // ワールド座標 → OBBローカル座標
    float localX = VDot(diff, axisX_);
    float localY = VDot(diff, axisY_);
    float localZ = VDot(diff, axisZ_);

    // OBB上の最近点を求める
    float closestX = localX;
    float closestY = localY;
    float closestZ = localZ;

    if (closestX < -halfSize_.x) closestX = -halfSize_.x;
    if (closestX > halfSize_.x) closestX = halfSize_.x;

    if (closestY < -halfSize_.y) closestY = -halfSize_.y;
    if (closestY > halfSize_.y) closestY = halfSize_.y;

    if (closestZ < -halfSize_.z) closestZ = -halfSize_.z;
    if (closestZ > halfSize_.z) closestZ = halfSize_.z;

    float diffX = localX - closestX;
    float diffY = localY - closestY;
    float diffZ = localZ - closestZ;

    float distSq =
        diffX * diffX +
        diffY * diffY +
        diffZ * diffZ;

    // 半径より遠ければ当たっていない
    if (distSq >= radius * radius)
    {
        return false;
    }

    // 押し出し方向
    VECTOR pushDir;

    if (distSq > 0.0001f)
    {
        float dist = sqrtf(distSq);

        pushDir =
            VAdd(
                VAdd(
                    VScale(axisX_, diffX / dist),
                    VScale(axisY_, diffY / dist)
                ),
                VScale(axisZ_, diffZ / dist)
            );

        float pushPower = radius - dist;

        pos = VAdd(pos, VScale(pushDir, pushPower));
    }
    else
    {
        // 完全に中に入っていた場合は、とりあえず上に押し出す
        pos.y += radius;
    }

    return true;
}

bool OBBCollider::IsHitSegment(VECTOR start, VECTOR end) const
{
    // ワールド座標をOBBローカル座標へ変換
    VECTOR startDiff = VSub(start, center_);
    VECTOR endDiff = VSub(end, center_);

    VECTOR localStart = VGet(
        VDot(startDiff, axisX_),
        VDot(startDiff, axisY_),
        VDot(startDiff, axisZ_)
    );

    VECTOR localEnd = VGet(
        VDot(endDiff, axisX_),
        VDot(endDiff, axisY_),
        VDot(endDiff, axisZ_)
    );

    VECTOR dir = VSub(localEnd, localStart);

    float tMin = 0.0f;
    float tMax = 1.0f;

    auto CheckAxis = [&](float startValue, float dirValue, float minValue, float maxValue) -> bool
        {
            if (fabsf(dirValue) < 0.0001f)
            {
                // 線分がこの軸方向にほぼ動いていない場合
                return startValue >= minValue && startValue <= maxValue;
            }

            float t1 = (minValue - startValue) / dirValue;
            float t2 = (maxValue - startValue) / dirValue;

            if (t1 > t2)
            {
                float temp = t1;
                t1 = t2;
                t2 = temp;
            }

            if (t1 > tMin) tMin = t1;
            if (t2 < tMax) tMax = t2;

            return tMin <= tMax;
        };

    if (!CheckAxis(localStart.x, dir.x, -halfSize_.x, halfSize_.x)) return false;
    if (!CheckAxis(localStart.y, dir.y, -halfSize_.y, halfSize_.y)) return false;
    if (!CheckAxis(localStart.z, dir.z, -halfSize_.z, halfSize_.z)) return false;

    return true;
}