#include "SoundManager.h"
#include "ResourceManager.h"
#include "../Application.h"
#include <DxLib.h>
#include <string>
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

int SoundManager::Create3DSE(SE se, float radius)
{
    std::string path;

    switch (se)
    {
    case SE::E_WALK:
        path = Application::PATH_SOUND + "Se/EnemyWalk.mp3";
        break;

    case SE::WALK:
        path = Application::PATH_SOUND + "Se/Walk.mp3";
        break;

    default:
        return -1;
    }

    // Ç±ÇÃå„Ç…ì«Ç›çûÇﬁâπÇ3DÉTÉEÉìÉhÇ∆ÇµÇƒçÏÇÈ
    SetCreate3DSoundFlag(TRUE);

    int handle = LoadSoundMem(path.c_str());

    // ïKÇ∏ñﬂÇ∑
    SetCreate3DSoundFlag(FALSE);

    if (handle == -1)
    {

        return -1;
    }

    Set3DRadiusSoundMem(radius, handle);
    ChangeVolumeSoundMem(seVolume_, handle);


    return handle;
}

void SoundManager::Delete3DSE(int& handle)
{
    if (handle != -1)
    {
        DeleteSoundMem(handle);
        handle = -1;
    }
}

void SoundManager::Play3DSE(int handle, const VECTOR& pos)
{
    if (handle == -1)
    {
        return;
    }

    Set3DPositionSoundMem(pos, handle);

    ChangeVolumeSoundMem(seVolume_, handle);

    PlaySoundMem(
        handle,
        DX_PLAYTYPE_BACK,
        TRUE
    );
}

void SoundManager::Set3DListener(const VECTOR& pos, const VECTOR& target)
{
    Set3DSoundOneMetre(100.0f);

    Set3DSoundListenerPosAndFrontPos_UpVecY(
        pos,
        target
    );
}


void SoundManager::Set3DSERadius(int handle, float radius)
{
    if (handle == -1)
    {
        return;
    }

    Set3DRadiusSoundMem(radius, handle);
}
