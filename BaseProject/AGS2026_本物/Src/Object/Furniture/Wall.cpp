#include "Wall.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Wall::Wall(const Transform* trans)
    : Furniture(NAME::WALL, trans) {
}

// ---------------------------------------------------------
// 壁パラメータ
// ---------------------------------------------------------
float wallH = 200.0f;   // 高さ
float wallW = 300.0f;   // 横幅
float wallD = 20.0f;    // 厚み

float slideX = 0.0f;
float slideY = 0.0f;
float slideZ = 0.0f;
// ---------------------------------------------------------

// 初期化
void Wall::Init() {
    colliders_.clear();

    VECTOR pos = trans_.GetPos();

    VECTOR center = VAdd(pos, VGet(slideX, slideY + wallH / 2.0f, slideZ));

    // 壁は1つのBoxでOK
    colliders_.push_back(BoxCollider(
        center,
        VGet(wallW / 2.0f, wallH / 2.0f, wallD / 2.0f)
    ));
}

// 更新
void Wall::Update(void) {
#ifdef _DEBUG
    auto& ins = InputManager::GetInstance();
    bool changed = false;

    // Wキーで操作
    if (CheckHitKey(KEY_INPUT_W)) {

        // Shift → 位置移動
        if (CheckHitKey(KEY_INPUT_LSHIFT)) {
            if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { slideX += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_LEFT)) { slideX -= 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_UP)) { slideZ += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_DOWN)) { slideZ -= 1.0f; changed = true; }
        }
        // 通常 → サイズ変更
        else {
            if (ins.IsTrgDown(KEY_INPUT_UP)) { wallH += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_DOWN)) { wallH -= 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { wallW += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_LEFT)) { wallW -= 1.0f; changed = true; }
        }
    }

    if (changed) Init();
#endif
}

// 描画
void Wall::Draw(void) {
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);

    // 当たり判定表示
    for (const auto& box : colliders_) {
        box.DrawDebug(GetColor(255, 0, 0));
    }
}