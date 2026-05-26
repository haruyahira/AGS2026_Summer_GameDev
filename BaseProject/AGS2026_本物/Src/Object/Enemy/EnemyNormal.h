#pragma once
#include "EnemyBase.h"
#include <vector>
#include <memory>

class Player;
class AnimationController;

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
    // アニメーション種別
    enum class ANIM_TYPE
    {
        IDLE,
        RUN,
        FAST_RUN,
    };

	void InitAnimation(void);
    bool CheckPlayerAttack(Player* player);
	std::unique_ptr<AnimationController> animationController_;
    void UpdateWander(Player* player);
    void DecideNextTarget();
    bool IsHitPlayer(Player* player);
    bool CollisionFurniture(Player* player, VECTOR beforePos);

    bool IsPlayerInView(Player* player);
    void UpdateChase(Player* player);
    void DrawViewRange();
    bool IsBlockedByWall(Player* player);
   

    // 追跡状態
    bool isChasing_;

    // 視野
    float viewRange_;
    float viewHalfAngleRad_;

    // 敵の正面方向
    VECTOR forwardDir_;


    // 巡回用
    std::vector<VECTOR> patrolPoints_;
    std::vector<std::vector<int>> patrolLinks_;
    int targetIndex_;

    float speed_;
    float waitTimer_;
    bool isWaiting_;

    // 当たり判定
    float radius_;

    // 床高さ固定用
    float groundY_;

    VECTOR waitBaseDir_;
    
	ANIM_TYPE animType_;

    bool isDead_ = false;
};