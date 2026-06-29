#include "Locker.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Locker::Locker(const Transform* trans)
    : Furniture(NAME::BOOKSLF, trans) {
}

// 初期化
void Locker::Init() {
    colliders_.clear();


}

// 更新
void Locker::Update(void) {
}

void Locker::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);
}
