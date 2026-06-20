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
float wallH = 373.0f;   // 高さ
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
    center_ = VAdd(
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

    axisX_ = VGet(c, 0.0f, -s);
    axisZ_ = VGet(s, 0.0f, c);

    VECTOR axisY = VGet(0.0f, 1.0f, 0.0f);

    obbCollider_ = OBBCollider(
        center_,
        VGet(w / 2.0f, h / 2.0f, d / 2.0f),
        axisX_,
        axisY,
        axisZ_
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


#ifdef _DEBUG
        // OBB当たり判定表示
        obbCollider_.DrawDebug(GetColor(255, 0, 0));
#endif
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

void Wall::DrawOutline(unsigned int color, bool isVerticalOnly) const
{
    obbCollider_.DrawOutline(color, isVerticalOnly);
}

VECTOR Wall::GetPos() const
{
    return center_;
}
VECTOR Wall::GetAxisX() const
{
    return axisX_;
}

VECTOR Wall::GetAxisZ() const
{
    return axisZ_;

}
VECTOR Wall::GetHalfSize() const
{

    float w =
        wallW * trans_.scl.x;

    float h =
        wallH * trans_.scl.y;

    float d =
        wallD * trans_.scl.z;

    return VGet(
        w * 0.5f,
        h * 0.5f,
        d * 0.5f
    );

}

void Wall::DrawDebug(unsigned int color) const
{
    obbCollider_.DrawDebug(color);
}