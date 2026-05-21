#pragma once
#include "Furniture.h"

class Wall : public Furniture {
public:
    // コンストラクタ
    Wall(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};