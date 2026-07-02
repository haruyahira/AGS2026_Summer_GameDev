#pragma once
#include "Furniture.h"

class Book : public Furniture {
public:
    // コンストラクタ
    Book(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};