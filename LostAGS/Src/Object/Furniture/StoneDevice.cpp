#include "StoneDevice.h"

#include "../../Manager/InputManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Utility/AsoUtility.h"
#include "../Player.h"

StoneDevice::StoneDevice(Player* player)
    : player_(player)
    , resMng_(ResourceManager::GetInstance())
{
    isActive_ = true;
    isSinking_ = false;
    sinkLength_ = 0.0f;
    startTime_ = 0;
}

StoneDevice::~StoneDevice(void)
{
    ResourceManager::GetInstance().DeleteDuplicateModel(transform_.modelId);
}

void StoneDevice::Init(void)
{
    transform_.SetModel(
        resMng_.LoadModelDuplicate(ResourceManager::SRC::RETURN_POINT));

    // 装置の位置
    // 好きな場所に調整してください
    transform_.pos = { -3500.0f, -100.0f,  400.0f };

    // 装置の大きさ
    transform_.scl = { 1.0f, 1.0f, 1.0f };

    // 装置の向き
    transform_.quaRot = Quaternion::Euler(
        0.0f,
        AsoUtility::Deg2RadF(0.0f),
        0.0f
    );

    transform_.Update();

    isActive_ = true;
    isSinking_ = false;
    sinkLength_ = 0.0f;

    // ここから5分カウント開始
    startTime_ = GetNowCount();
}

void StoneDevice::Update(void)
{ 
if (!isActive_)
{
    return;
}

int nowTime = GetNowCount();
int elapsedTime = nowTime - startTime_;
//
//// 5分経過したら沈み始める
//if (elapsedTime >= LIMIT_TIME_MS && !isSinking_)
//{
//    StartSinking();
//}

// 沈んでいる間
if (isSinking_)
{
    UpdateSinking();
    return;
}
}

void StoneDevice::Draw(void)
{
    if (!isActive_)
    {
        return;
    }

    MV1DrawModel(transform_.modelId);

}

bool StoneDevice::IsActive(void) const
{
    return isActive_;
}

void StoneDevice::StartSinking(void)
{
    isSinking_ = true;
}

void StoneDevice::UpdateSinking(void)
{
    sinkLength_ += SINK_SPEED;

    // 下に沈める
    transform_.pos.y -= SINK_SPEED;
    transform_.Update();

    // 一定距離沈んだら完全に無効化
    if (sinkLength_ >= SINK_END_LENGTH)
    {
        isActive_ = false;
        isSinking_ = false;
    }
}

bool StoneDevice::IsNearPlayer(void) const
{
    if (player_ == nullptr)
    {
        return false;
    }

    VECTOR playerPos = player_->GetPos();
    VECTOR devicePos = transform_.pos;

    // 高さは無視して、XZ平面で距離を見る
    float dx = playerPos.x - devicePos.x;
    float dz = playerPos.z - devicePos.z;

    float distSq = dx * dx + dz * dz;
    float rangeSq = CHECK_RANGE * CHECK_RANGE;

    return distSq <= rangeSq;
}

void StoneDevice::DrawGuide(void) const
{

   /* DrawString(
        20,
        80,
        "Fキーでアイテム登録 / Eキーで脱出",
        GetColor(255, 255, 255)
    );*/

}

void StoneDevice::DrawTimer(void) const
{
    int nowTime = GetNowCount();
    int elapsedTime = nowTime - startTime_;

    int remainTime = LIMIT_TIME_MS - elapsedTime;

    if (remainTime < 0)
    {
        remainTime = 0;
    }

    int remainSec = remainTime / 1000;
    int minute = remainSec / 60;
    int second = remainSec % 60;
}

void StoneDevice::DrawUI(void) 
{

    if (!isSinking_ && IsNearPlayer())
    {
        DrawGuide();
    }

    DrawTimer();
}