#pragma once
#include <DxLib.h>
#include <string>
#include <unordered_map>
class ResourceManager;

class SoundManager
{
public:
    static SoundManager* GetInstance();

    void Load();                       // 音声リソース読み込み
    void PlayBGM(const std::string& name, bool loop = true);
    void StopBGM();
    void PlaySE(const std::string& name);
    void PlaySELoop(const std::string& name);
    void SetVolumeBGM(int volume);
    void SetVolumeSES(int volume);
    void SetVolumeSE(int volume, const std::string& name);
    void StopAllSE();
    void StopSE(const std::string& name);
    int  GetSEHandle(const std::string& name);

    void SetFrequencySE(const std::string& name, int freq);

    void Release();                    // メモリ解放

private:
    SoundManager();
    ~SoundManager() = default;

    int currentBGM_ = -1;  // 再生中のBGMハンドル
    std::unordered_map<std::string, int> bgmHandles_;
    std::unordered_map<std::string, int> seHandles_;

    // リソース管理
    ResourceManager* resMng_;
};
