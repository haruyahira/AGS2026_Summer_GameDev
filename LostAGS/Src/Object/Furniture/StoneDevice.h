#pragma once

#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../Common/Transform.h"

class Player;

class StoneDevice
{
public:

    StoneDevice(Player* player);
    ~StoneDevice(void);

    void Init(void);
    void Update(void);
    void Draw(void);
    void DrawUI(void);

    bool IsActive(void) const;
    bool IsNearPlayer(void) const;

private:

    void StartSinking(void);
    void UpdateSinking(void);
   
    void DrawGuide(void) const;
    void DrawTimer(void) const;

  

private:

    Player* player_;

    ResourceManager& resMng_;

    Transform transform_;

    bool isActive_;
    bool isSinking_;

    float sinkLength_;

    int startTime_;

    // 制限時間：5分
    static constexpr int LIMIT_TIME_MS = 5 * 60 * 1000;

    // プレイヤーが装置を起動できる距離
    static constexpr float CHECK_RANGE = 120.0f;

    // 沈む速度
    static constexpr float SINK_SPEED = 1.5f;

    // どれだけ沈んだら完全に消えた扱いにするか
    static constexpr float SINK_END_LENGTH = 180.0f;
};