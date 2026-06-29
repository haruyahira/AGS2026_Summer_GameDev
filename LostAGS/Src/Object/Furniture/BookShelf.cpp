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

void BookShelf::Draw(void)
{
    trans_.Update();

    // 本棚の中心位置をデバッグ表示
    DrawSphere3D(
        trans_.pos,
        30.0f,
        16,
        GetColor(255, 0, 0),
        GetColor(255, 0, 0),
        TRUE
    );

    // モデル描画
    MV1DrawModel(trans_.modelId);
}
