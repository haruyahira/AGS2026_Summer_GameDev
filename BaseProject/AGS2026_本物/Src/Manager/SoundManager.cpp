#include "SoundManager.h"

SoundManager* SoundManager::instance_ = nullptr;

////////////////////////////////////////////////////////////
// インスタンス取得
////////////////////////////////////////////////////////////
SoundManager& SoundManager::GetInstance()
{
    if (instance_ == nullptr)
    {
        instance_ = new SoundManager();
    }

    return *instance_;
}

////////////////////////////////////////////////////////////
// 初期化
////////////////////////////////////////////////////////////
void SoundManager::Init()
{
    bgms_.clear();
    ses_.clear();
}

////////////////////////////////////////////////////////////
// BGM読み込み
////////////////////////////////////////////////////////////
bool SoundManager::LoadBGM(
    const std::string& name,
    const char* path)
{
    int handle = LoadSoundMem(path);

    if (handle == -1)
    {
        return false;
    }

    bgms_[name].handle = handle;

    return true;
}

////////////////////////////////////////////////////////////
// SE読み込み
////////////////////////////////////////////////////////////
bool SoundManager::LoadSE(
    const std::string& name,
    const char* path)
{
    int handle = LoadSoundMem(path);

    if (handle == -1)
    {
        return false;
    }

    ses_[name].handle = handle;

    return true;
}

////////////////////////////////////////////////////////////
// BGM再生
////////////////////////////////////////////////////////////
void SoundManager::PlayBGM(
    const std::string& name,
    bool loop)
{
    if (bgms_.count(name) == 0) return;

    int playType =
        loop ?
        DX_PLAYTYPE_LOOP :
        DX_PLAYTYPE_BACK;

    PlaySoundMem(
        bgms_[name].handle,
        playType);

    SetFrequencySoundMem(
        static_cast<int>(
            44100 * bgms_[name].speed),
        bgms_[name].handle);
}

////////////////////////////////////////////////////////////
// SE再生
////////////////////////////////////////////////////////////
void SoundManager::PlaySE(
    const std::string& name)
{
    if (ses_.count(name) == 0) return;

    PlaySoundMem(
        ses_[name].handle,
        DX_PLAYTYPE_BACK,
        TRUE);

    SetFrequencySoundMem(
        static_cast<int>(
            44100 * ses_[name].speed),
        ses_[name].handle);
}

////////////////////////////////////////////////////////////
// BGM停止
////////////////////////////////////////////////////////////
void SoundManager::StopBGM(
    const std::string& name)
{
    if (bgms_.count(name) == 0) return;

    StopSoundMem(
        bgms_[name].handle);
}

////////////////////////////////////////////////////////////
// 全BGM停止
////////////////////////////////////////////////////////////
void SoundManager::StopAllBGM()
{
    for (auto& bgm : bgms_)
    {
        StopSoundMem(
            bgm.second.handle);
    }
}

////////////////////////////////////////////////////////////
// 全SE停止
////////////////////////////////////////////////////////////
void SoundManager::StopAllSE()
{
    for (auto& se : ses_)
    {
        StopSoundMem(
            se.second.handle);
    }
}

////////////////////////////////////////////////////////////
// BGM音量
////////////////////////////////////////////////////////////
void SoundManager::SetBGMVolume(
    const std::string& name,
    int volume)
{
    if (bgms_.count(name) == 0) return;

    ChangeVolumeSoundMem(
        volume,
        bgms_[name].handle);
}

////////////////////////////////////////////////////////////
// SE音量
////////////////////////////////////////////////////////////
void SoundManager::SetSEVolume(
    const std::string& name,
    int volume)
{
    if (ses_.count(name) == 0) return;

    ChangeVolumeSoundMem(
        volume,
        ses_[name].handle);
}

////////////////////////////////////////////////////////////
// BGM速度
////////////////////////////////////////////////////////////
void SoundManager::SetBGMSpeed(
    const std::string& name,
    float speed)
{
    if (bgms_.count(name) == 0) return;

    bgms_[name].speed = speed;

    SetFrequencySoundMem(
        static_cast<int>(44100 * speed),
        bgms_[name].handle);
}

////////////////////////////////////////////////////////////
// SE速度
////////////////////////////////////////////////////////////
void SoundManager::SetSESpeed(
    const std::string& name,
    float speed)
{
    if (ses_.count(name) == 0) return;

    ses_[name].speed = speed;

    SetFrequencySoundMem(
        static_cast<int>(44100 * speed),
        ses_[name].handle);
}

////////////////////////////////////////////////////////////
// BGM再生中？
////////////////////////////////////////////////////////////
bool SoundManager::IsPlayingBGM(
    const std::string& name)
{
    if (bgms_.count(name) == 0)
    {
        return false;
    }

    return CheckSoundMem(
        bgms_[name].handle) == 1;
}

////////////////////////////////////////////////////////////
// 解放
////////////////////////////////////////////////////////////
void SoundManager::Release()
{
    for (auto& bgm : bgms_)
    {
        DeleteSoundMem(
            bgm.second.handle);
    }

    for (auto& se : ses_)
    {
        DeleteSoundMem(
            se.second.handle);
    }

    bgms_.clear();
    ses_.clear();
}

////////////////////////////////////////////////////////////
// SEループ再生
////////////////////////////////////////////////////////////
void SoundManager::PlaySELoop(
    const std::string& name)
{
    if (ses_.count(name) == 0) return;

    if (CheckSoundMem(ses_[name].handle) == 1)
    {
        return;
    }

    PlaySoundMem(
        ses_[name].handle,
        DX_PLAYTYPE_LOOP);
}

////////////////////////////////////////////////////////////
// SE停止
////////////////////////////////////////////////////////////
void SoundManager::StopSE(
    const std::string& name)
{
    if (ses_.count(name) == 0) return;

    StopSoundMem(
        ses_[name].handle);
}

////////////////////////////////////////////////////////////
// SE再生中？
////////////////////////////////////////////////////////////
bool SoundManager::IsPlayingSE(
    const std::string& name)
{
    if (ses_.count(name) == 0)
    {
        return false;
    }

    return CheckSoundMem(
        ses_[name].handle) == 1;
}