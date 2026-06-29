#include "Freezer.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Freezer::Freezer(const Transform* trans)
    : Furniture(NAME::BOOKSLF, trans) {
}

// 初期化
void Freezer::Init() {
    colliders_.clear();


}

// 更新
void Freezer::Update(void) {
}

void Freezer::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);
}
