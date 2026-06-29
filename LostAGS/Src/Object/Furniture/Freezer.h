#pragma once
#include "Furniture.h"

class Freezer : public Furniture {
public:
    // コンストラクタ
    Freezer(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};