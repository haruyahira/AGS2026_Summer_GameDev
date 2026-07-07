#pragma once
#include "Furniture.h"

class Shelf : public Furniture {
public:
    // コンストラクタ
    Shelf(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};