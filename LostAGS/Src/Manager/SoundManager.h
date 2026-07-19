#pragma once
#include <DxLib.h>
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
        E_WALK,
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
    int GetBGMVolume() const;

    //====================
    // SE
    //====================

    void PlaySE(SE se);

    void SetSEVolume(int volume);
    int GetSEVolume() const;

    void SetSEPitch(float pitch);
    void SetSEPlaySpeed(float speed, SE se);

    void StopSE(SE se);
    void StopAllSound();

    int Create3DSE(SE se, float radius);
    void Delete3DSE(int& handle);
    void Play3DSE(int handle, const VECTOR& pos);
    void Set3DListener(const VECTOR& pos, const VECTOR& target);
    void Set3DSERadius(int handle, float radius);

    void SetBGMVolumeScale(float scale);
    void SetSEVolumeScale(float scale);

    float GetBGMVolumeScale() const;
    float GetSEVolumeScale() const;
  

  
   
private:

    SoundManager();

    std::map<BGM, int> bgms_;
    std::map<SE, int> ses_;

    int currentBGM_ = -1;

    int bgmVolume_ = 255;
    int seVolume_ = 255;

    float sePitch_ = 1.0f;

    float bgmVolumeScale_;
    float seVolumeScale_;
};