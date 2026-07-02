#include "Book.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Book::Book(const Transform* trans)
    : Furniture(NAME::BOOKSLF, trans) {
}

// 初期化
void Book::Init() {
    colliders_.clear();


}

// 更新
void Book::Update(void) {
}

void Book::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);
}
