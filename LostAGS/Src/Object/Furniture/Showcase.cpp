#include "Showcase.h"
#include "../../Manager/InputManager.h"

// ---------------------------------------------------------
// ショーケースの当たり判定サイズ
// モデルに合わせて調整してください
// ---------------------------------------------------------
static float showcaseW = 785.0f;   // 横幅
static float showcaseH = 170.0f;   // 高さ
static float showcaseD = 180.0f;   // 奥行き

static float showcaseSlideX = 0.0f;
static float showcaseSlideY = 0.0f;
static float showcaseSlideZ = 0.0f;

// コンストラクタ
Showcase::Showcase(const Transform* trans, float rotY)
    : Furniture(NAME::SHOWCASE, trans)
{
    rotY_ = rotY;
}

// 初期化
void Showcase::Init()
{
    colliders_.clear();

    VECTOR pos = trans_.GetPos();

    float w = showcaseW * trans_.scl.x;
    float h = showcaseH * trans_.scl.y;
    float d = showcaseD * trans_.scl.z;

    center_ = VAdd(
        pos,
        VGet(
            showcaseSlideX,
            showcaseSlideY + h / 2.0f,
            showcaseSlideZ
        )
    );

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
void Showcase::Update(void)
{
}

// 描画
void Showcase::Draw(void)
{
    trans_.Update();

    MV1DrawModel(trans_.modelId);

#ifdef _DEBUG
    obbCollider_.DrawDebug(GetColor(255, 0, 0));
#endif
}

bool Showcase::ResolveCollision(
    VECTOR& pos,
    float radius,
    float bottomY,
    float topY
)
{
    return obbCollider_.ResolveCollisionXZ(
        pos,
        radius,
        bottomY,
        topY
    );
}

bool Showcase::ResolveCameraCollision(
    VECTOR& cameraPos,
    float radius
)
{
    return obbCollider_.ResolveSphere(
        cameraPos,
        radius
    );
}

bool Showcase::IsBlockingSight(
    VECTOR start,
    VECTOR end
) const
{
    return obbCollider_.IsHitSegment(start, end);
}