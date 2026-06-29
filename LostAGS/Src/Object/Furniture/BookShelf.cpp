#include "BookShelf.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
BookShelf::BookShelf(const Transform* trans)
    : Furniture(NAME::BOOKSLF, trans) {
}

// 初期化
void BookShelf::Init() {
    colliders_.clear();


}

// 更新
void BookShelf::Update(void) {
}

// 描画
void BookShelf::Draw(void) {
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);

    // 当たり判定表示
    for (const auto& box : colliders_) {
        box.DrawDebug(GetColor(255, 0, 0));
    }
}