#include "SoundManager.h"
#include "ResourceManager.h"
#include <DxLib.h>
#include <algorithm>

SoundManager& SoundManager::GetInstance()
{
    static SoundManager instance;
    return instance;
}
SoundManager::SoundManager()
{
    currentBGM_ = -1;

    bgmVolume_ = 255;
    seVolume_ = 255;

    sePitch_ = 1.0f;
}

void SoundManager::Init()
{
    auto& res = ResourceManager::GetInstance();

    // BGM
    bgms_[BGM::GAME] =
        res.Load(ResourceManager::SRC::GAME_BGM).handleId_;

    bgms_[BGM::TITLE] =
        res.Load(ResourceManager::SRC::TITLE_BGM).handleId_;

    bgms_[BGM::CHASE] =
        res.Load(ResourceManager::SRC::CHASE_BGM).handleId_; 
    
    bgms_[BGM::CLEAR] =
        res.Load(ResourceManager::SRC::CLEAR_BGM).handleId_;


    // SE
    ses_[SE::WALK] =
        res.Load(ResourceManager::SRC::WALK_SE).handleId_;  
    ses_[SE::E_WALK] =
        res.Load(ResourceManager::SRC::WALK_E_SE).handleId_;  
  
    ses_[SE::RUN] =
        res.Load(ResourceManager::SRC::WALK_SE).handleId_;  
    ses_[SE::ATTACK] =
        res.Load(ResourceManager::SRC::ATTACK_SE).handleId_;  
    ses_[SE::HIT] =
        res.Load(ResourceManager::SRC::HIT_SE).handleId_;
    ses_[SE::DISE] =
        res.Load(ResourceManager::SRC::DISC_SE).handleId_;
    ses_[SE::DOOROP] =
        res.Load(ResourceManager::SRC::DOOROP_SE).handleId_;
}

void SoundManager::PlayBGM(BGM bgm, bool loop)
{
    int handle = bgms_[bgm];

    // ìØÇ∂BGMÇ»ÇÁçƒê∂ÇµÇ»Ç¢
    if (currentBGM_ == handle)
    {
        return;
    }

    StopBGM();

    currentBGM_ = handle;

    ChangeVolumeSoundMem(bgmVolume_, handle);

    PlaySoundMem(
        handle,
        loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
}

void SoundManager::StopBGM()
{
    for (auto& bgm : bgms_)
    {
        StopSoundMem(bgm.second);
    }

    currentBGM_ = -1;
}

void SoundManager::SetBGMVolume(int volume)
{
    bgmVolume_ = volume;
}

void SoundManager::PlaySE(SE se)
{
    int handle = ses_[se];

    StopSoundMem(handle);

    ChangeVolumeSoundMem(seVolume_, handle);

    PlaySoundMem(handle, DX_PLAYTYPE_BACK);
}

void SoundManager::SetSEVolume(int volume)
{
    seVolume_ = volume;
}

void SoundManager::SetSEPitch(float pitch)
{
    sePitch_ = pitch;
}

void SoundManager::SetBGMPlaySpeed(float speed, BGM bgm)
{
    int handle = bgms_[bgm];

    SetFrequencySoundMem(
        static_cast<int>(44100 * speed),
        handle);
}

void SoundManager::SetSEPlaySpeed(float speed, SE se)
{
    speed = std::clamp(speed, 0.1f, 4.0f);

    int handle = ses_[se];

    int baseFreq = GetFrequencySoundMem(handle);

    SetFrequencySoundMem(
        static_cast<int>(baseFreq * speed),
        handle);
}
void SoundManager::StopSE(SE se)
{
    StopSoundMem(ses_[se]);
}

void SoundManager::StopAllSound()
{
    //====================
    // BGMí‚é~
    //====================
    for (auto& bgm : bgms_)
    {
        StopSoundMem(bgm.second);
    }

    //====================
    // SEí‚é~
    //====================
    for (auto& se : ses_)
    {
        StopSoundMem(se.second);
    }

    currentBGM_ = -1;
}
