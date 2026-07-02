#pragma once
#include "Furniture.h"

class Locker : public Furniture
{
public:
    Locker(const Transform* trans);

    void Init() override;
    void Update(void) override;
    void Draw(void) override;

private:
    int animAttachIndex_ = -1;     // アタッチしたアニメーション番号
    float animTime_ = 0.0f;        // 現在のアニメーション時間
    float animTotalTime_ = 0.0f;   // アニメーションの総時間
    float animSpeed_ = 0.5f;       // アニメーション速度
};