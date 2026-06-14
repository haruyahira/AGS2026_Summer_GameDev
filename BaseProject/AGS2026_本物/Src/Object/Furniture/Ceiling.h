#pragma once
#include "Furniture.h"
#include "../Furniture/LightBlocker.h"
#include "../Collider/OBBCollider.h"

class Ceiling
    : public Furniture
    , public LightBlocker {
public:
    // コンストラクタ
    Ceiling(const Transform* trans, float rotY);

    void Init() override;
    void Update() override;
    void Draw() override;
    bool ResolveCollision(
        VECTOR& pos,
        float radius,
        float bottomY,
        float topY) override;

    bool ResolveCameraCollision(
        VECTOR& cameraPos,
        float radius) override;

    VECTOR GetPos() const override;

    VECTOR GetAxisX() const override;

    VECTOR GetAxisZ() const override;

    VECTOR GetHalfSize() const override;
    void DrawDebug(unsigned int color) const override;

private:
    OBBCollider obbCollider_;

    // 壁のY回転
    float rotY_;

};