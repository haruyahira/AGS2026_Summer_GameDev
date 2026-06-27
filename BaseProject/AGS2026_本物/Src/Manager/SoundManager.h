#pragma once
#include <DxLib.h>
#include <string>
#include <unordered_map>

class SoundManager
{
private:

    struct Sound
    {
        int handle = -1;
        float speed = 1.0f;
    };

    std::unordered_map<std::string, Sound> bgms_;
    std::unordered_map<std::string, Sound> ses_;

    //--------------------------------------------------
    // シングルトン
    //--------------------------------------------------
    static SoundManager* instance_;

    SoundManager() {}

public:

    //--------------------------------------------------
    // インスタンス取得
    //--------------------------------------------------
    static SoundManager& GetInstance();

    //--------------------------------------------------
    // 初期化
    //--------------------------------------------------
    void Init();

    //--------------------------------------------------
    // 読み込み
    //--------------------------------------------------
    bool LoadBGM(
        const std::string& name,
        const char* path);

    bool LoadSE(
        const std::string& name,
        const char* path);

    //--------------------------------------------------
    // 再生
    //--------------------------------------------------
    void PlayBGM(
        const std::string& name,
        bool loop = true);

    void PlaySE(
        const std::string& name);

    //--------------------------------------------------
    // 停止
    //--------------------------------------------------
    void StopBGM(
        const std::string& name);

    void StopAllBGM();

    void StopAllSE();

    //--------------------------------------------------
    // 音量
    //--------------------------------------------------
    void SetBGMVolume(
        const std::string& name,
        int volume);

    void SetSEVolume(
        const std::string& name,
        int volume);

    //--------------------------------------------------
    // 再生速度
    //--------------------------------------------------
    void SetBGMSpeed(
        const std::string& name,
        float speed);

    void SetSESpeed(
        const std::string& name,
        float speed);

    //--------------------------------------------------
    // 再生中？
    //--------------------------------------------------
    bool IsPlayingBGM(
        const std::string& name);

    void PlaySELoop(const std::string& name);
    void StopSE(const std::string& name);
    bool IsPlayingSE(const std::string& name);

    //--------------------------------------------------
    // 解放
    //--------------------------------------------------
    void Release();
};