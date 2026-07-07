#include "Shelf.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Shelf::Shelf(const Transform* trans)
    : Furniture(NAME::BOOKSLF, trans) {
}

// 初期化
void Shelf::Init() {
    colliders_.clear();


}

// 更新
void Shelf::Update(void) {
}

void Shelf::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);
}
