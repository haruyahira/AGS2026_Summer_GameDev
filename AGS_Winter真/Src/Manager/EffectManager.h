#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "DxLib.h"

class Transform;
class SceneManager;
class ResourceManager;

class EffectManager
{
public:
    struct EffectControl
    {
        int handle;
        float time;
        float stopTime;
        bool stopped;
    };

    EffectManager();
    ~EffectManager();

    void Initialize();
    void Load();

    // エフェクト再生（位置固定）
    int Play(const std::string& name, const VECTOR& pos);

    // Transform に追従して再生
    int PlayFollow(const std::string& name, Transform* target);

    // 更新＆描画
    void Update();
    void Draw();

    int PlayAndStopAt(const char* name, const VECTOR& pos, float stopTime);

    void SetSpeed(int handle, float speed);

    void SetTime(int handle, float time);

    std::vector<EffectControl> effectControls_;

private:
    SceneManager& scnMng_;
    ResourceManager* resMng_;

    struct FollowData {
        int handle;
        Transform* target;
        float remainingTime; // 秒
    };

    std::unordered_map<std::string, int> effectHandles;

    // 追従エフェクトの一覧
    std::vector<FollowData> followEffects_;

    std::unordered_map<std::string, float> effectDurations_; // name -> seconds

    static const int EffectLimit = 2000;

    int h;
};
