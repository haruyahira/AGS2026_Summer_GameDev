#pragma once
#include "EnemyBase.h"
#include <vector>

class Player;

class EnemyNormal : public EnemyBase
{
public:
    EnemyNormal(void);
    ~EnemyNormal(void);

    void Init(void) override;
    void Update(void) override;
    void Update(Player* player) override;
    void Draw(void) override;

    void SetPatrolPoints(const std::vector<VECTOR>& points);
    void SetPatrolLinks(const std::vector<std::vector<int>>& links);

private:
    void UpdateWander(Player* player);
    void DecideNextTarget();
    bool IsHitPlayer(Player* player);
    bool CollisionFurniture(Player* player, VECTOR beforePos);

    bool IsPlayerInView(Player* player);
    void UpdateChase(Player* player);
    void DrawViewRange();
    bool IsBlockedByWall(Player* player);
   

    // ’ÇÕó‘Ô
    bool isChasing_;

    // ‹–ì
    float viewRange_;
    float viewHalfAngleRad_;

    // “G‚Ì³–Ê•ûŒü
    VECTOR forwardDir_;


    // „‰ñ—p
    std::vector<VECTOR> patrolPoints_;
    std::vector<std::vector<int>> patrolLinks_;
    int targetIndex_;

    float speed_;
    float waitTimer_;
    bool isWaiting_;

    // “–‚½‚è”»’è
    float radius_;

    // °‚‚³ŒÅ’è—p
    float groundY_;

    VECTOR waitBaseDir_;
};