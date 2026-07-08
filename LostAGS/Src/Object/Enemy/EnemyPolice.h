#pragma once
#include "EnemyBase.h"
#include <vector>

class Player;

class EnemyPolice : public EnemyBase
{
public:
    EnemyPolice(void);
    virtual ~EnemyPolice(void);

    void Init(void) override;
    void Draw(void) override;

    void SetPatrolPoints(const std::vector<VECTOR>& points);
    void SetPatrolLinks(const std::vector<std::vector<int>>& links);
    void SetPos(const VECTOR& pos);
private:
    enum class ANIM_TYPE
    {
        IDLE,
        RUN,
        HIT_R,
        HIT_L,
    };

    void InitAnimation(void) override;

    void UpdateWander(Player* player) override;
    void UpdateChase(Player* player) override;

    void StartAttack(Player* player) override;
    void UpdateAttack(Player* player) override;
    VECTOR GetAttackPos(void) const override;

    int GetAnimType(void) const override;

    void DecideNextTarget(void);

    void InitAttackFrame(void);
    void PlayAnimation(ANIM_TYPE animType, bool isLoop = true);
private:
    std::vector<VECTOR> patrolPoints_;
    std::vector<std::vector<int>> patrolLinks_;

    int targetIndex_;

    float waitTimer_;
    bool isWaiting_;

    VECTOR waitBaseDir_;

    ANIM_TYPE animType_;

    int rightHandFrame_;
    int leftHandFrame_;

    bool isRightAttack_;

};
