
#pragma once
#include "Furniture.h"

class Locker : public Furniture {
public:
    // コンストラクタ
    Locker(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};