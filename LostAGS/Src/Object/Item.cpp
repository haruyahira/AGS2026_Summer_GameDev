#include "Item.h"

#include "../Utility/AsoUtility.h"
#include "../Shader/RimLightEffect.h"

Item::Item()
    : resMng_(ResourceManager::GetInstance())
{
    type_ = TYPE::LAPTOP;
    isActive_ = false;

    pickupRange_ = 168.0f;
    viewDot_ = 0.7f;
}

Item::~Item()
{

    if (transform_.modelId != -1)
    {
        ResourceManager::GetInstance().DeleteDuplicateModel(transform_.modelId);
    }

}

void Item::Init(
    TYPE type,
    ResourceManager::SRC modelSrc,
    VECTOR pos,
    VECTOR scl,
    VECTOR rot
)
{
    type_ = type;
    isActive_ = true;

    transform_.SetModel(resMng_.LoadModelDuplicate(modelSrc));
    transform_.pos = pos;
    transform_.scl = scl;

    transform_.quaRot =
        Quaternion::Euler(
            rot.x,
            rot.y,
            rot.z
        );

    transform_.Update();
}

void Item::Update()
{
    if (!isActive_)
    {
        return;
    }

    transform_.Update();
}

void Item::Draw()
{
    if (!isActive_)
    {
        return;
    }

    transform_.Update();
    MV1DrawModel(transform_.modelId);
}


bool Item::IsActive() const
{
    return isActive_;
}

void Item::SetActive(bool active)
{
    isActive_ = active;
}

Item::TYPE Item::GetType() const
{
    return type_;
}

const char* Item::GetName() const
{
    switch (type_)
    {
    case TYPE::LAPTOP:
        return "ノートPC";

    case TYPE::WATCH:
        return "腕時計";

    case TYPE::BOOK168:
        return "I68Book";

    case TYPE::MEDICINE:
        return "薬";

    default:
        return "不明なアイテム";
    }
}

VECTOR Item::GetPos() const
{
    return transform_.pos;
}

bool Item::IsInPlayerView(const VECTOR& playerPos, const VECTOR& playerForward) const
{
    if (!isActive_)
    {
        return false;
    }

    VECTOR toItem = VSub(transform_.pos, playerPos);

    // 高さ差を無視
    toItem.y = 0.0f;

    float distance = VSize(toItem);

    if (distance > pickupRange_)
    {
        return false;
    }

    return true;
}



void Item::Pickup()
{
    isActive_ = false;
}

float Item::GetPickupRange(void) const
{
    return pickupRange_;
}


void Item::DrawRimLight(RimLightEffect& rimLight)
{
    if (!isActive_)
    {
        return;
    }

    VECTOR color = VGet(0.25f, 0.75f, 1.0f);

    switch (type_)
    {
    case TYPE::LAPTOP:
        color = VGet(0.25f, 0.75f, 1.0f);
        break;

    case TYPE::WATCH:
        color = VGet(1.0f, 0.85f, 0.25f);
        break;

    case TYPE::BOOK168:
        color = VGet(1.0f, 1.0f, 1.0f);
        break;

    case TYPE::MEDICINE:
        color = VGet(1.0f, 0.35f, 0.55f);
        break;

    default:
        break;
    }

    float t = GetNowCount() / 1000.0f;
    float blink =
        (sinf(t * 4.0f) + 1.0f) * 0.5f;


    float power = 5.0f;
    float intensity = 0.45f + blink * 0.25f;
    float alpha = 0.6f;


    rimLight.Begin(
        color,
        power,
        intensity,
        alpha
    );

    transform_.Update();
    MV1DrawModel(transform_.modelId);

    rimLight.End();
}

void Item::DrawWhiteBlink(void)
{
    if (!isActive_)
    {
        return;
    }

    float t = GetNowCount() / 1000.0f;

    // 0.0 ～ 1.0
    float blink =
        (sinf(t * 6.0f) + 1.0f) * 0.5f;

    // 完全に消える瞬間も作る
    int alpha =
        (int)(blink * 180.0f);

    if (alpha <= 5)
    {
        return;
    }

    transform_.Update();

    // 重要：通常シェーダーを解除してから描く
    SetUseVertexShader(-1);
    SetUsePixelShader(-1);

    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);
    SetUseTextureToShader(2, -1);

    SetUseLighting(FALSE);

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);

    SetUseBackCulling(TRUE);

    // 白く加算描画
    SetDrawBright(255, 255, 255);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    MV1DrawModel(transform_.modelId);

}
