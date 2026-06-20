#include "Showcase.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Showcase::Showcase(const Transform* trans)
    : Furniture(NAME::SHOWCASE, trans) {
}

// 初期化
void Showcase::Init() {
    colliders_.clear();

   
}

// 更新
void Showcase::Update(void) {
}

// 描画
void Showcase::Draw(void) {
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);

    // 当たり判定表示
    for (const auto& box : colliders_) {
        box.DrawDebug(GetColor(255, 0, 0));
    }
}