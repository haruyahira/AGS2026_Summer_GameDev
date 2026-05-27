#pragma once
#include "EnemyBase.h"
#include <vector>

class Player;

class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal(void);
    virtual ~EnemyNormal(void);

    void Init(void) override;
    void Draw(void) override;

    void SetPatrolPoints(const std::vector<VECTOR>& points);
    void SetPatrolLinks(const std::vector<std::vector<int>>& links);

private:
    enum class ANIM_TYPE
    {
        IDLE,
        RUN,
        FAST_RUN,
    };

    void InitAnimation(void) override;
    void UpdateWander(Player* player) override;
    void UpdateChase(Player* player) override;

    int GetAnimType(void) const override;

    void DecideNextTarget(void);

private:
    std::vector<VECTOR> patrolPoints_;
    std::vector<std::vector<int>> patrolLinks_;

    int targetIndex_;

    float waitTimer_;
    bool isWaiting_;

    VECTOR waitBaseDir_;

    ANIM_TYPE animType_;
};