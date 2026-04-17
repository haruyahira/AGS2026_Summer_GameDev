#include <EffekseerForDXLib.h>
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Object/Common/Transform.h"
#include "EffectManager.h"

EffectManager::EffectManager()
    :
    scnMng_(SceneManager::GetInstance())
    
{
    Initialize();
resMng_ = &ResourceManager::GetInstance();
    Load();
}

EffectManager::~EffectManager()
{
    for (auto& ef : effectHandles)
    {
        DeleteEffekseerEffect(ef.second);
    }
}

void EffectManager::Initialize()
{
  /*  SetUseDirect3DVersion(DX_DIRECT3D_11);

    if (Effekseer_Init(EffectLimit) == -1)
    {
        DxLib_End();
    }*/

   
    SetChangeScreenModeGraphicsSystemResetFlag(FALSE);
    Effekseer_SetGraphicsDeviceLostCallbackFunctions();

  /*  SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);*/
}

void EffectManager::Load()
{
   // effectHandles["bakuhatu"] = resMng_->Load(
      //  ResourceManager::SRC::BAKUHATU).handleId_;
    //effectHandles["bakuhatu"] = LoadEffekseerEffect("Data/Effect/Flame.efk", 50.0f);
    effectHandles["bakuhatu"] = LoadEffekseerEffect("Data/Effect/Flame.efk", 120.0f);
    effectDurations_["bakuhatu"] = 2.5f;
    effectHandles["hit"] = LoadEffekseerEffect("Data/Effect/hit.efkefc", 50.0f);
    effectHandles["smoke"] = LoadEffekseerEffect("Data/Effect/smoke.efkefc", 80.0f);
}

int EffectManager::Play(const std::string& name, const VECTOR& pos)
{
    if (effectHandles.count(name) == 0) return -1;

    int handle = PlayEffekseer3DEffect(effectHandles[name]);
    SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);
    return handle;
}

int EffectManager::PlayFollow(const std::string& name, Transform* target)
{
    if (effectHandles.count(name) == 0) return -1;
    int handle = PlayEffekseer3DEffect(effectHandles[name]);
    float dur = 1.0f;
    if (effectDurations_.count(name)) dur = effectDurations_[name];

    followEffects_.push_back({ handle, target, dur });

    VECTOR p = target->GetPos();
    SetPosPlayingEffekseer3DEffect(handle, p.x, p.y, p.z);
    return handle;
}

void EffectManager::Update()
{
    //for (auto it = followEffects_.begin(); it != followEffects_.end(); )
    //{
    //    it->remainingTime -= scnMng_.GetDeltaTime();
    //    if (it->remainingTime <= 0.0f)
    //    {
    //        // 必要なら DeleteEffekseerEffect はしない（リソースは effectHandles で保持）
    //        it = followEffects_.erase(it);
    //        continue;
    //    }
    //    VECTOR p = it->target->GetPos();
    //    SetPosPlayingEffekseer3DEffect(it->handle, p.x, p.y, p.z);
    //    ++it;
    //}
    //UpdateEffekseer3D();
     // 追従エフェクト処理
    for (auto it = followEffects_.begin(); it != followEffects_.end(); )
    {
        it->remainingTime -= scnMng_.GetDeltaTime();
        if (it->remainingTime <= 0.0f)
        {
            it = followEffects_.erase(it);
            continue;
        }
        VECTOR p = it->target->GetPos();
        SetPosPlayingEffekseer3DEffect(it->handle, p.x, p.y, p.z);
        ++it;
    }

    // ★ 指定時間で停止させる制御 ★
    for (auto& e : effectControls_)
    {
        if (e.stopped) continue;

        e.time += scnMng_.GetDeltaTime();

        if (e.time >= e.stopTime)
        {
            SetSpeedPlayingEffekseer3DEffect(e.handle, 0.0f); // 完全停止
            e.stopped = true;
        }
    }

    UpdateEffekseer3D();

}


void EffectManager::Draw()
{
    DrawEffekseer3D();

   
}
int EffectManager::PlayAndStopAt(const char* name, const VECTOR& pos, float stopTime)
{
    int h = Play(name, pos);

    EffectControl ctrl;
    ctrl.handle = h;
    ctrl.time = 0.0f;
    ctrl.stopTime = stopTime;
    ctrl.stopped = false;

    effectControls_.push_back(ctrl);
    return h;
}