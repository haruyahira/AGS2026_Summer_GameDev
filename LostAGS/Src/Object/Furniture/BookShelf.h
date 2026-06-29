
#pragma once
#include "Furniture.h"

class BookShelf : public Furniture {
public:
    // コンストラクタ
    BookShelf(const Transform* trans);

    void Init() override;
    void Update() override;
    void Draw() override;
};