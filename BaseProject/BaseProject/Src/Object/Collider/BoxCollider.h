#pragma once

#include <DxLib.h>

class BoxCollider {

public:
    VECTOR center;    // 中心座標
    VECTOR halfSize;  // 中心から各面までの距離（サイズの半分）

    // コンストラクタ
    BoxCollider(VECTOR center, VECTOR size);

    // デバッグ用：箱をワイヤーフレームで描画する
    //void DrawDebug(unsigned int color) const;

    // 当たり判定：点との判定
    bool CheckCollision(VECTOR point) const;

    // 当たり判定：他のBoxとの判定（AABB同士）
    bool CheckCollision(const BoxCollider& other) const;

    VECTOR GetClosestPoint(VECTOR targetPos) const;

    float GetDistanceSq(VECTOR point) const;

    void DrawDebug(unsigned int color = 0xff0000) const; // 赤色をデフォルトに

private:

};