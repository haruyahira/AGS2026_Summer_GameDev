#include "BoxCollider.h"
#include <cmath>

// 値を min と max の間に収めるヘルパー関数
float Clamp(float n, float min, float max) {
    if (n < min) return min;
    if (n > max) return max;
    return n;
}

BoxCollider::BoxCollider(VECTOR center, VECTOR size) {
    this->center = center;
    // サイズを半分にして保持
    this->halfSize = VGet(size.x / 2.0f, size.y / 2.0f, size.z / 2.0f);
}

//void BoxCollider::DrawDebug(unsigned int color) const {
//    // 中心とhalfSizeから最小点(左下奥)と最大点(右上手前)を計算
//    VECTOR minPos = VSub(center, halfSize);
//    VECTOR maxPos = VAdd(center, halfSize);
//
//    // DXLib: 指定の2点からなる直方体を描画する
//    // 引数: 最小座標, 最大座標, 色, 塗りつぶしフラグ
//    DrawCube3D(minPos, maxPos, color, GetColor(255, 255, 255), FALSE);
//}

bool BoxCollider::CheckCollision(VECTOR point) const {
    if (std::abs(point.x - center.x) > halfSize.x) return false;
    if (std::abs(point.y - center.y) > halfSize.y) return false;
    if (std::abs(point.z - center.z) > halfSize.z) return false;

    return true;
}

bool BoxCollider::CheckCollision(const BoxCollider& other) const {
    if (std::abs(center.x - other.center.x) > (halfSize.x + other.halfSize.x)) return false;
    if (std::abs(center.y - other.center.y) > (halfSize.y + other.halfSize.y)) return false;
    if (std::abs(center.z - other.center.z) > (halfSize.z + other.halfSize.z)) return false;

    return true;
}

VECTOR BoxCollider::GetClosestPoint(VECTOR targetPos) const
{
    VECTOR res = targetPos;

    // center と halfSize を使って「箱の端」を計算
    float minX = center.x - halfSize.x;
    float maxX = center.x + halfSize.x;
    float minY = center.y - halfSize.y;
    float maxY = center.y + halfSize.y;
    float minZ = center.z - halfSize.z;
    float maxZ = center.z + halfSize.z;

    // targetPosを箱の範囲内に閉じ込める（クランプ）
    if (res.x < minX) res.x = minX;
    else if (res.x > maxX) res.x = maxX;

    if (res.y < minY) res.y = minY;
    else if (res.y > maxY) res.y = maxY;

    if (res.z < minZ) res.z = minZ;
    else if (res.z > maxZ) res.z = maxZ;

    return res;
}

float BoxCollider::GetDistanceSq(VECTOR point) const
{
    // 箱の最小座標と最大座標を計算
    VECTOR minPos = VSub(center, halfSize);
    VECTOR maxPos = VAdd(center, halfSize);

    // 点を箱の範囲内にクランプして「箱の表面上の最短地点」を割り出す
    float closestX = Clamp(point.x, minPos.x, maxPos.x);
    float closestY = Clamp(point.y, minPos.y, maxPos.y);
    float closestZ = Clamp(point.z, minPos.z, maxPos.z);

    // 元の点と「最短地点」の差分を計算
    float dx = point.x - closestX;
    float dy = point.y - closestY;
    float dz = point.z - closestZ;

    // 距離の2乗を返す (x^2 + y^2 + z^2)
    return (dx * dx + dy * dy + dz * dz);
}

void BoxCollider::DrawDebug(unsigned int color) const {
    // 箱の最小点と最大点を計算
    VECTOR minPos = VSub(center, halfSize);
    VECTOR maxPos = VAdd(center, halfSize);

    // DXライブラリの関数でワイヤーフレームの箱を描画
    DrawCube3D(minPos, maxPos, color, color, FALSE);
}