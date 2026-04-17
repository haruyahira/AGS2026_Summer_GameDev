#pragma once
#include <DxLib.h>

class CutIn
{
public:
    void Init();
    void Start();
    void Update();
    void Draw();
    bool IsActive() const { return active_; }

private:
    bool active_ = false;
    int timer_ = 0;

    int imgGlow_ = -1;
    int imgShadow_ = -1;
    int imgSpeedLine_ = -1;
    int imgCutin_ = -1;
    int imgSkillName_ = -1;
};
