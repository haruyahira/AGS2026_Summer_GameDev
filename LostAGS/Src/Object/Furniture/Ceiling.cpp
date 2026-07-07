#include "Ceiling.h"
#include "../../Manager/InputManager.h"
#include "../Furniture/LightBlocker.h"

// コンストラクタ
Ceiling::Ceiling(const Transform* trans, float rotY)
    : Furniture(NAME::CEILING, trans)
{
    rotY_ = rotY;
}

// ---------------------------------------------------------
// 床パラメータ
// ---------------------------------------------------------
static float floorW = 440.0f;  // 横幅
static float floorD = 400.0f;  // 奥行き
static float floorT = 20.0f;   // 厚み

static float slideX = 0.0f;
static float slideY = 0.0f;
static float slideZ = 0.0f;
// ---------------------------------------------------------

void Ceiling::Init()
{
    VECTOR pos = trans_.GetPos();

    // 床の実サイズ
    float w = floorW * trans_.scl.x;
    float h = floorT * trans_.scl.y;
    float d = floorD * trans_.scl.z;

    // 床の中心
    VECTOR center = VAdd(
        pos,
        VGet(
            slideX,
            slideY,
            slideZ
        )
    );

    // Y回転
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

void Ceiling::Update(void)
{
#ifdef _DEBUG
    auto& ins = InputManager::GetInstance();
    bool changed = false;

    // Cキーで操作などにした方がWallと被らなくて安全
    if (CheckHitKey(KEY_INPUT_C) || CheckHitKey(KEY_INPUT_RCONTROL))
    {
        // Shift → 位置移動
        if (CheckHitKey(KEY_INPUT_LSHIFT))
        {
            if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { slideX += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_LEFT)) { slideX -= 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_UP)) { slideZ += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_DOWN)) { slideZ -= 1.0f; changed = true; }
        }
        else
        {
            if (ins.IsTrgDown(KEY_INPUT_UP)) { floorD += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_DOWN)) { floorD -= 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { floorW += 1.0f; changed = true; }
            if (ins.IsTrgDown(KEY_INPUT_LEFT)) { floorW -= 1.0f; changed = true; }
        }
    }

    if (changed)
    {
        Init();
    }
#endif
}

void Ceiling::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);


#ifdef _DEBUG
    // OBB当たり判定表示
    obbCollider_.DrawDebug(GetColor(255, 0, 0));
#endif
}

bool Ceiling::ResolveCollision(
    VECTOR& pos,
    float radius,
    float bottomY,
    float topY)
{
    return obbCollider_.ResolveCollisionBottom(
        pos,
        radius,
        bottomY,
        topY);
}

bool Ceiling::ResolveCameraCollision(
    VECTOR& cameraPos,
    float radius)
{
    return obbCollider_.ResolveSphere(
        cameraPos,
        radius);
}

VECTOR Ceiling::GetPos() const
{
    return trans_.pos;
}

VECTOR Ceiling::GetAxisX() const
{
    return trans_.GetRight();
}

VECTOR Ceiling::GetAxisZ() const
{
    return trans_.GetForward();
}

VECTOR Ceiling::GetHalfSize() const
{
    return VGet(
        trans_.scl.x * 500.0f,
        30.0f,
        trans_.scl.z * 500.0f
    );
}
void Ceiling::DrawDebug(unsigned int color) const
{
}