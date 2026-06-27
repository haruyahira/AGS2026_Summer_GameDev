
#pragma once
#include "Furniture.h"

class Showcase : public Furniture {
public:
    // コンストラクタ
    Showcase(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};