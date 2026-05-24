#include "Wall.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Wall::Wall(const Transform* trans, float rotY)
    : Furniture(NAME::WALL, trans) {
    rotY_ = rotY;
}

// ---------------------------------------------------------
// 壁パラメータ
// ---------------------------------------------------------
float wallH = 400.0f;   // 高さ
float wallW = 440.0f;   // 横幅
float wallD = 60.0f;    // 厚み

float slideX = 0.0f;
float slideY = 0.0f;
float slideZ = 0.0f;
// ---------------------------------------------------------

// 初期化
void Wall::Init() {


    VECTOR pos = trans_.GetPos();

    // 壁の実サイズ
    float w = wallW * trans_.scl.x;
    float h = wallH * trans_.scl.y;
    float d = wallD * trans_.scl.z;

    // 壁の中心
    VECTOR center = VAdd(
        pos,
        VGet(
            slideX,
            slideY + h / 2.0f,
            slideZ
        )
    );

    // Y回転を使う
    float c = cosf(rotY_);
    float s = sinf(rotY_);

    VECTOR axisX = VGet(c, 0.0f, -s);
    VECTOR axisY = VGet(0.0f, 1.0f, 0.0f);
    VECTOR axisZ = VGet(s, 0.0f, c);

    obbCollider_ = OBBCollider(
        center,
        VGet(w / 2.0f, h / 2.0f, d / 2.0f),
        axisX,
        axisY,
        axisZ
    );


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

    {
        trans_.Update();

        // モデル描画
        MV1DrawModel(trans_.modelId);

        // OBB当たり判定表示
        obbCollider_.DrawDebug(GetColor(255, 0, 0));
    }

}

bool Wall::ResolveCollision(
    VECTOR& pos,
    float radius,
    float bottomY,
    float topY)
{
    return obbCollider_.ResolveCollisionXZ(
        pos,
        radius,
        bottomY,
        topY);
}

bool Wall::ResolveCameraCollision(
    VECTOR& cameraPos,
    float radius)
{
    return obbCollider_.ResolveSphere(
        cameraPos,
        radius);
}

bool Wall::IsBlockingSight(VECTOR start, VECTOR end) const
{
    return obbCollider_.IsHitSegment(start, end);
}