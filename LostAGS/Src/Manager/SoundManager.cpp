#include "SoundManager.h"
#include "ResourceManager.h"
#include "../Application.h"

#include <DxLib.h>
#include <string>
#include <algorithm>

namespace
{
    // Šî–{‰¹—Ê‚Æ”{—¦‚©‚çADXƒ‰ƒCƒuƒ‰ƒŠ—p‚Ì‰¹—Ê‚ğŒvZ
    int CalculateActualVolume(
        int baseVolume,
        float volumeScale)
    {
        int actualVolume =
            static_cast<int>(
                static_cast<float>(baseVolume) *
                volumeScale
                );

        return std::clamp(
            actualVolume,
            0,
            255
        );
    }
}

SoundManager& SoundManager::GetInstance()
{
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager()
{
    currentBGM_ = -1;

    // 255‚¾‚Æ1.0”{‚Å‚·‚Å‚ÉÅ‘å‰¹—Ê‚É‚È‚é‚½‚ßA
    // 1.1”{ˆÈã‚ğg‚¢‚½‚¢ê‡‚Í180`200’ö“x‚ª‚¨‚·‚·‚ß
    bgmVolume_ = 180;
    seVolume_ = 200;

    // ‰Šú”{—¦‚Í1.0”{
    bgmVolumeScale_ = 1.0f;
    seVolumeScale_ = 1.0f;

    sePitch_ = 1.0f;
}

void SoundManager::Init()
{
    ResourceManager& res =
        ResourceManager::GetInstance();

    // ========================================
    // BGM
    // ========================================

    bgms_[BGM::GAME] =
        res.Load(
            ResourceManager::SRC::GAME_BGM
        ).handleId_;

    bgms_[BGM::TITLE] =
        res.Load(
            ResourceManager::SRC::TITLE_BGM
        ).handleId_;

    bgms_[BGM::CHASE] =
        res.Load(
            ResourceManager::SRC::CHASE_BGM
        ).handleId_;

    bgms_[BGM::CLEAR] =
        res.Load(
            ResourceManager::SRC::CLEAR_BGM
        ).handleId_;

    // ========================================
    // SE
    // ========================================

    ses_[SE::WALK] =
        res.Load(
            ResourceManager::SRC::WALK_SE
        ).handleId_;

    ses_[SE::E_WALK] =
        res.Load(
            ResourceManager::SRC::WALK_E_SE
        ).handleId_;

    ses_[SE::RUN] =
        res.Load(
            ResourceManager::SRC::WALK_SE
        ).handleId_;

    ses_[SE::ATTACK] =
        res.Load(
            ResourceManager::SRC::ATTACK_SE
        ).handleId_;

    ses_[SE::HIT] =
        res.Load(
            ResourceManager::SRC::HIT_SE
        ).handleId_;

    ses_[SE::DISE] =
        res.Load(
            ResourceManager::SRC::DISC_SE
        ).handleId_;

    ses_[SE::DOOROP] =
        res.Load(
            ResourceManager::SRC::DOOROP_SE
        ).handleId_;

    // ========================================
    // “Ç‚İ‚İÏ‚İƒTƒEƒ“ƒh‚ÖŒ»İ‚Ì‰¹—Ê‚ğ”½‰f
    // ========================================

    const int actualBGMVolume =
        CalculateActualVolume(
            bgmVolume_,
            bgmVolumeScale_
        );

    for (auto& bgm : bgms_)
    {
        if (bgm.second != -1)
        {
            ChangeVolumeSoundMem(
                actualBGMVolume,
                bgm.second
            );
        }
    }

    const int actualSEVolume =
        CalculateActualVolume(
            seVolume_,
            seVolumeScale_
        );

    for (auto& se : ses_)
    {
        if (se.second != -1)
        {
            ChangeVolumeSoundMem(
                actualSEVolume,
                se.second
            );
        }
    }
}

void SoundManager::PlayBGM(
    BGM bgm,
    bool loop)
{
    auto found =
        bgms_.find(bgm);

    if (found == bgms_.end())
    {
        return;
    }

    const int handle =
        found->second;

    if (handle == -1)
    {
        return;
    }

    // “¯‚¶BGM‚ªÄ¶’†‚È‚çÄ¶‚µ’¼‚³‚È‚¢
    if (currentBGM_ == handle &&
        CheckSoundMem(handle) == 1)
    {
        return;
    }

    StopBGM();

    currentBGM_ = handle;

    const int actualVolume =
        CalculateActualVolume(
            bgmVolume_,
            bgmVolumeScale_
        );

    ChangeVolumeSoundMem(
        actualVolume,
        currentBGM_
    );

    PlaySoundMem(
        currentBGM_,
        loop
        ? DX_PLAYTYPE_LOOP
        : DX_PLAYTYPE_BACK,
        TRUE
    );
}

void SoundManager::StopBGM()
{
    for (auto& bgm : bgms_)
    {
        if (bgm.second != -1)
        {
            StopSoundMem(
                bgm.second
            );
        }
    }

    currentBGM_ = -1;
}

void SoundManager::SetBGMVolume(
    int volume)
{
    // Šî–{‰¹—Ê‚ğ0`255‚É§ŒÀ
    bgmVolume_ =
        std::clamp(
            volume,
            0,
            255
        );

    const int actualVolume =
        CalculateActualVolume(
            bgmVolume_,
            bgmVolumeScale_
        );

    // Ä¶’†‚ÌBGM‚Ö‘¦”½‰f
    if (currentBGM_ != -1)
    {
        ChangeVolumeSoundMem(
            actualVolume,
            currentBGM_
        );
    }

    // “Ç‚İ‚İÏ‚İ‚Ì‘SBGM‚É‚à”½‰f
    for (auto& bgm : bgms_)
    {
        if (bgm.second != -1)
        {
            ChangeVolumeSoundMem(
                actualVolume,
                bgm.second
            );
        }
    }
}

int SoundManager::GetBGMVolume() const
{
    return bgmVolume_;
}

void SoundManager::SetBGMVolumeScale(
    float scale)
{
    // ‰¹—Ê”{—¦‚ğ0.0`2.0”{‚É§ŒÀ
    bgmVolumeScale_ =
        std::clamp(
            scale,
            0.0f,
            2.0f
        );

    const int actualVolume =
        CalculateActualVolume(
            bgmVolume_,
            bgmVolumeScale_
        );

    // Ä¶’†‚ÌBGM‚Ö‘¦”½‰f
    if (currentBGM_ != -1)
    {
        ChangeVolumeSoundMem(
            actualVolume,
            currentBGM_
        );
    }

    // “Ç‚İ‚İÏ‚İ‚Ì‘SBGM‚É‚à”½‰f
    for (auto& bgm : bgms_)
    {
        if (bgm.second != -1)
        {
            ChangeVolumeSoundMem(
                actualVolume,
                bgm.second
            );
        }
    }
}

float SoundManager::GetBGMVolumeScale() const
{
    return bgmVolumeScale_;
}

void SoundManager::SetBGMPlaySpeed(
    float speed,
    BGM bgm)
{
    speed =
        std::clamp(
            speed,
            0.1f,
            4.0f
        );

    auto found =
        bgms_.find(bgm);

    if (found == bgms_.end())
    {
        return;
    }

    const int handle =
        found->second;

    if (handle == -1)
    {
        return;
    }

    SetFrequencySoundMem(
        static_cast<int>(
            44100.0f * speed
            ),
        handle
    );
}

void SoundManager::PlaySE(SE se)
{
    auto found =
        ses_.find(se);

    if (found == ses_.end())
    {
        return;
    }

    const int handle =
        found->second;

    if (handle == -1)
    {
        return;
    }

    StopSoundMem(handle);

    const int actualVolume =
        CalculateActualVolume(
            seVolume_,
            seVolumeScale_
        );

    ChangeVolumeSoundMem(
        actualVolume,
        handle
    );

    PlaySoundMem(
        handle,
        DX_PLAYTYPE_BACK,
        TRUE
    );
}

void SoundManager::StopSE(SE se)
{
    auto found =
        ses_.find(se);

    if (found == ses_.end())
    {
        return;
    }

    const int handle =
        found->second;

    if (handle != -1)
    {
        StopSoundMem(handle);
    }
}

void SoundManager::SetSEVolume(
    int volume)
{
    // Šî–{‰¹—Ê‚ğ0`255‚É§ŒÀ
    seVolume_ =
        std::clamp(
            volume,
            0,
            255
        );

    const int actualVolume =
        CalculateActualVolume(
            seVolume_,
            seVolumeScale_
        );

    // “Ç‚İ‚İÏ‚İ‚Ì‘SSE‚Ö‘¦”½‰f
    for (auto& se : ses_)
    {
        if (se.second != -1)
        {
            ChangeVolumeSoundMem(
                actualVolume,
                se.second
            );
        }
    }
}

int SoundManager::GetSEVolume() const
{
    return seVolume_;
}

void SoundManager::SetSEVolumeScale(
    float scale)
{
    // ‰¹—Ê”{—¦‚ğ0.0`2.0”{‚É§ŒÀ
    seVolumeScale_ =
        std::clamp(
            scale,
            0.0f,
            2.0f
        );

    const int actualVolume =
        CalculateActualVolume(
            seVolume_,
            seVolumeScale_
        );

    // “Ç‚İ‚İÏ‚İ‚Ì‘SSE‚Ö‘¦”½‰f
    for (auto& se : ses_)
    {
        if (se.second != -1)
        {
            ChangeVolumeSoundMem(
                actualVolume,
                se.second
            );
        }
    }
}

float SoundManager::GetSEVolumeScale() const
{
    return seVolumeScale_;
}

void SoundManager::SetSEPitch(
    float pitch)
{
    sePitch_ =
        std::clamp(
            pitch,
            0.1f,
            4.0f
        );
}

void SoundManager::SetSEPlaySpeed(
    float speed,
    SE se)
{
    speed =
        std::clamp(
            speed,
            0.1f,
            4.0f
        );

    auto found =
        ses_.find(se);

    if (found == ses_.end())
    {
        return;
    }

    const int handle =
        found->second;

    if (handle == -1)
    {
        return;
    }

    const int baseFrequency =
        GetFrequencySoundMem(handle);

    if (baseFrequency <= 0)
    {
        return;
    }

    SetFrequencySoundMem(
        static_cast<int>(
            static_cast<float>(baseFrequency) *
            speed
            ),
        handle
    );
}

void SoundManager::StopAllSound()
{
    // ========================================
    // BGM’â~
    // ========================================

    for (auto& bgm : bgms_)
    {
        if (bgm.second != -1)
        {
            StopSoundMem(
                bgm.second
            );
        }
    }

    // ========================================
    // SE’â~
    // ========================================

    for (auto& se : ses_)
    {
        if (se.second != -1)
        {
            StopSoundMem(
                se.second
            );
        }
    }

    currentBGM_ = -1;
}

int SoundManager::Create3DSE(
    SE se,
    float radius)
{
    std::string path;

    switch (se)
    {
    case SE::E_WALK:
        path =
            Application::PATH_SOUND +
            "Se/EnemyWalk.mp3";
        break;

    case SE::WALK:
        path =
            Application::PATH_SOUND +
            "Se/Walk.mp3";
        break;

    default:
        return -1;
    }

    // Ÿ‚É“Ç‚İ‚ŞƒTƒEƒ“ƒh‚ğ3DƒTƒEƒ“ƒh‚É‚·‚é
    SetCreate3DSoundFlag(TRUE);

    const int handle =
        LoadSoundMem(
            path.c_str()
        );

    // •K‚¸’Êí‚Ì“Ç‚İ‚İ‚Ö–ß‚·
    SetCreate3DSoundFlag(FALSE);

    if (handle == -1)
    {
        return -1;
    }

    // ”¼Œa‚ªƒ}ƒCƒiƒX‚É‚È‚ç‚È‚¢‚æ‚¤‚É‚·‚é
    if (radius < 0.0f)
    {
        radius = 0.0f;
    }

    Set3DRadiusSoundMem(
        radius,
        handle
    );

    const int actualVolume =
        CalculateActualVolume(
            seVolume_,
            seVolumeScale_
        );

    ChangeVolumeSoundMem(
        actualVolume,
        handle
    );

    return handle;
}

void SoundManager::Delete3DSE(
    int& handle)
{
    if (handle == -1)
    {
        return;
    }

    StopSoundMem(handle);
    DeleteSoundMem(handle);

    handle = -1;
}

void SoundManager::Play3DSE(
    int handle,
    const VECTOR& pos)
{
    if (handle == -1)
    {
        return;
    }

    Set3DPositionSoundMem(
        pos,
        handle
    );

    const int actualVolume =
        CalculateActualVolume(
            seVolume_,
            seVolumeScale_
        );

    // Ä¶‚ÉŒ»İ‚ÌŠî–{‰¹—Ê‚Æ”{—¦‚ğ”½‰f
    ChangeVolumeSoundMem(
        actualVolume,
        handle
    );

    PlaySoundMem(
        handle,
        DX_PLAYTYPE_BACK,
        TRUE
    );
}

void SoundManager::Set3DListener(
    const VECTOR& pos,
    const VECTOR& target)
{
    Set3DSoundOneMetre(
        100.0f
    );

    Set3DSoundListenerPosAndFrontPos_UpVecY(
        pos,
        target
    );
}

void SoundManager::Set3DSERadius(
    int handle,
    float radius)
{
    if (handle == -1)
    {
        return;
    }

    if (radius < 0.0f)
    {
        radius = 0.0f;
    }

    Set3DRadiusSoundMem(
        radius,
        handle
    );
}