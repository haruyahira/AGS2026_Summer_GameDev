#include "CeilingLight.h"
#include <DxLib.h>
#include <math.h>

CeilingLight::CeilingLight(
    const Transform* trans)
{
    trans_ =
        std::make_shared<Transform>(*trans);
}

CeilingLight::~CeilingLight()
{
}

void CeilingLight::Init()
{
}

void CeilingLight::Update()
{
    trans_->Update();
}

void CeilingLight::Draw()
{
    // 通常描画
    MV1DrawModel(trans_->modelId);

    // モデル自体も少し発光して見えるように重ね描き
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);

    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        80
    );

    SetDrawBright(
        255,
        235,
        190
    );

    MV1DrawModel(trans_->modelId);

    SetDrawBright(
        255,
        255,
        255
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    SetWriteZBuffer3D(TRUE);
}

static void DrawGlowDisc(
    VECTOR center,
    float radius,
    int div,
    int color)
{
    VECTOR prev =
        VGet(
            center.x + radius,
            center.y,
            center.z
        );

    for (int i = 1; i <= div; i++)
    {
        float angle =
            DX_TWO_PI_F * i / div;

        VECTOR current =
            VGet(
                center.x + cosf(angle) * radius,
                center.y,
                center.z + sinf(angle) * radius
            );

        DrawTriangle3D(
            center,
            prev,
            current,
            color,
            TRUE
        );

        prev = current;
    }
}

void CeilingLight::DrawGlow()
{
    VECTOR pos =
        trans_->GetPos();

    // 天井の下面に発光面が見えるように少し下げる
    pos.y -= 4.0f;

    // 重要：
    // Zバッファは使う。壁の奥なら見えなくする。
    // ただしZ書き込みはしない。
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);
    SetUseBackCulling(FALSE);

    // 外側のやわらかい光
    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        45
    );

    DrawGlowDisc(
        pos,
        18.0f,
        32,
        GetColor(255, 210, 130)
    );

    // 中央の白い発光面
    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        160
    );

    DrawGlowDisc(
        pos,
        9.0f,
        32,
        GetColor(255, 245, 220)
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    SetUseBackCulling(TRUE);
    SetWriteZBuffer3D(TRUE);
}
VECTOR CeilingLight::GetPos() const
{
    return trans_->GetPos();
}