#include "Item.h"

#include "../Utility/AsoUtility.h"

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

void Item::Init(TYPE type, ResourceManager::SRC modelSrc, VECTOR pos, VECTOR scl)
{
    type_ = type;
    isActive_ = true;

    transform_.SetModel(resMng_.LoadModelDuplicate(modelSrc));
    transform_.pos = pos;
    transform_.scl = scl;
    transform_.quaRot = Quaternion();
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

    case TYPE::KEY:
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
