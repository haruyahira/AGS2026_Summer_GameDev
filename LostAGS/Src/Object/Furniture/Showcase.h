#pragma once

#include "Furniture.h"
#include "../Collider/OBBCollider.h"

class Showcase : public Furniture
{
public:

    Showcase(const Transform* trans, float rotY = 0.0f);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

    bool ResolveCollision(
        VECTOR& pos,
        float radius,
        float bottomY,
        float topY
    ) override;

    bool ResolveCameraCollision(
        VECTOR& cameraPos,
        float radius
    ) override;

    bool IsBlockingSight(
        VECTOR start,
        VECTOR end
    ) const override;

private:

    float rotY_;

    OBBCollider obbCollider_;

    VECTOR center_;
    VECTOR axisX_;
    VECTOR axisZ_;
};
