#pragma once
#include <map>

class SoundManager
{
public:

    enum class BGM
    {
        GAME,
        TITLE,
        CHASE,
        CLEAR
    };

    enum class SE
    {
        WALK,
        RUN,
        ATTACK,
        HIT,
        DISE,
        DOOROP,
    };

public:

    static SoundManager& GetInstance();

    void Init();

    //====================
    // BGM
    //====================

    void PlayBGM(BGM bgm, bool loop = true);
    void StopBGM();

    void SetBGMVolume(int volume);
    void SetBGMPlaySpeed(float speed, BGM bgm);

    //====================
    // SE
    //====================

    void PlaySE(SE se);

    void SetSEVolume(int volume);

    void SetSEPitch(float pitch);
    void SetSEPlaySpeed(float speed, SE se);

    void StopSE(SE se);
    void StopAllSound();
private:

    SoundManager();

    std::map<BGM, int> bgms_;
    std::map<SE, int> ses_;

    int currentBGM_ = -1;

    int bgmVolume_ = 255;
    int seVolume_ = 255;

    float sePitch_ = 1.0f;
};