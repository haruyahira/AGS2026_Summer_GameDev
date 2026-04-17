#include "../Manager/ResourceManager.h"
#include "SoundManager.h"
SoundManager::SoundManager()
{
    resMng_ = &ResourceManager::GetInstance();
    currentBGM_ = -1;
}

SoundManager* SoundManager::GetInstance()
{
    static SoundManager instance;
    return &instance;
}

void SoundManager::Load()
{
    // ---- BGM読み込み ----

    // タイトルBGM
    bgmHandles_["op"] = resMng_->Load(ResourceManager::SRC::OP).handleId_;
    // ゲームクリアBGM
    bgmHandles_["ed"] = LoadSoundMem("Data/BGM/ed.wav");
    // ゲームBGM
    bgmHandles_["bgm1"] = resMng_->Load(ResourceManager::SRC::BGM1).handleId_;
    bgmHandles_["bgm2"] = resMng_->Load(ResourceManager::SRC::BGM2).handleId_;

    // SE
    // Walk
    seHandles_["walk"] = resMng_->Load(ResourceManager::SRC::WALK).handleId_;
    seHandles_["run"] = resMng_->Load(ResourceManager::SRC::RUN).handleId_;


    // ---- SE読み込み ----
   /* seHandles_["shot"] = LoadSoundMem("Data/Sound/SE/shot.wav");*/
  /*  seHandles_["explosion"] = LoadSoundMem("Data/Sound/SE/explosion.wav");*/
   /* seHandles_["click"] = LoadSoundMem("Data/Sound/SE/click.wav");*/
}

void SoundManager::PlayBGM(const std::string& name, bool loop)
{
    if (bgmHandles_.count(name) == 0) return;

    // 現在再生中なら止める
    if (currentBGM_ != -1) StopSoundMem(currentBGM_);

    currentBGM_ = bgmHandles_[name];
    PlaySoundMem(currentBGM_, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
}

void SoundManager::StopBGM()
{
    if (currentBGM_ != -1)
    {
        StopSoundMem(currentBGM_);
        currentBGM_ = -1;
    }
}

void SoundManager::PlaySE(const std::string& name)
{
    if (seHandles_.count(name) == 0) return;
    PlaySoundMem(seHandles_[name], DX_PLAYTYPE_BACK);
}

void SoundManager::PlaySELoop(const std::string& name)
{
    if (seHandles_.count(name) == 0) return;
    if (CheckSoundMem(seHandles_[name]) == 1) return; // 再生中なら何もしない
    PlaySoundMem(seHandles_[name], DX_PLAYTYPE_LOOP);
}

void SoundManager::StopAllSE()
{
    for (auto& se : seHandles_)
    {
        StopSoundMem(se.second);
    }
}

void SoundManager::SetVolumeBGM(int volume)
{
    if (currentBGM_ != -1)
        ChangeVolumeSoundMem(volume, currentBGM_);
}

void SoundManager::SetVolumeSES(int volume)
{
    for (auto& se : seHandles_)
    {
        ChangeVolumeSoundMem(volume, se.second);
    }
}
void SoundManager::SetVolumeSE(int volume, const std::string& name )
{
    // 読み込んだSEから指定名のサウンドを検索
    if (seHandles_.count(name) == 0) return;

    ChangeVolumeSoundMem(volume, seHandles_[name]);
}

void SoundManager::StopSE(const std::string& name)
{
    if (seHandles_.count(name) == 0) return;
    StopSoundMem(seHandles_[name]);
}

int SoundManager::GetSEHandle(const std::string& name)
{
    if (seHandles_.count(name) == 0) return -1;
    return seHandles_[name];
}

void SoundManager::SetFrequencySE(const std::string& name, int freq)
{
    if (seHandles_.count(name) == 0) return;
    SetFrequencySoundMem(freq, seHandles_[name]);
}
//void SoundManager::Release()
//{
//    for (auto& bgm : bgmHandles_) DeleteSoundMem(bgm.second);
//    for (auto& se : seHandles_)  DeleteSoundMem(se.second);
//
//    bgmHandles_.clear();
//    seHandles_.clear();
//}
