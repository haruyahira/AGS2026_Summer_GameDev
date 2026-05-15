#pragma once
#include "Furniture.h"

class Table : public Furniture {
public:
    // コンストラクタで、親クラスに必要な情報を渡しつつ、自分用のコライダを作る
    Table(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};