#pragma once
#include <DxLib.h>

class OBBCollider
{
public:

    OBBCollider();

    OBBCollider(
        VECTOR center,
        VECTOR halfSize,
        VECTOR axisX,
        VECTOR axisY,
        VECTOR axisZ);

    // 点との当たり判定
    bool CheckCollision(VECTOR point) const;


    // 点からOBBまでの距離の2乗
    float GetDistanceSq(VECTOR point) const;

    // 一番近い点
    VECTOR GetClosestPoint(VECTOR point) const;


    // デバッグ描画
    void DrawDebug(unsigned int color) const;

    bool ResolveCollisionXZ(
        VECTOR& pos,
        float radius,
        float bottomY,
        float topY) const;


    bool ResolveCollisionTop(
        VECTOR& pos,
        float radius,
        float bottomY,
        float topY) const;


    bool ResolveCollisionBottom(
        VECTOR& pos,
        float radius,
        float bottomY,
        float topY) const;

private:

    // 中心座標
    VECTOR center_;

    // 半分サイズ
    VECTOR halfSize_;

    // OBBのローカル軸
    VECTOR axisX_;
    VECTOR axisY_;
    VECTOR axisZ_;
};