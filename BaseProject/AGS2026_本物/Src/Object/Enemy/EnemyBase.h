#pragma once
#include "../ActorBase.h"
#include <memory>

class Player;
class AnimationController;

class EnemyBase : public ActorBase
{
public:
    EnemyBase(void);
    virtual ~EnemyBase(void);

    virtual void Init(void) = 0;

    // ActorBase用
    virtual void Update(void) override;

    // Playerを使う敵更新
    virtual void Update(Player* player);

    // 共通描画
    virtual void Draw(void) override;

protected:
    // 敵ごとの個性
    virtual void UpdateWander(Player* player) = 0;
    virtual void UpdateChase(Player* player) = 0;
    virtual void InitAnimation(void) = 0;

    // 派生クラス側の現在アニメーション番号を返す
    virtual int GetAnimType(void) const = 0;

    // 共通処理
    bool CheckPlayerAttack(Player* player);
    bool IsHitPlayer(Player* player);
    bool CollisionFurniture(Player* player, VECTOR beforePos);

    bool IsPlayerInView(Player* player);
    bool IsBlockedByWall(Player* player);

    void DrawViewRange(void);

protected:
    std::unique_ptr<AnimationController> animationController_;

    float speed_;
    float radius_;
    float groundY_;

    bool isDead_;
    bool isChasing_;

    float viewRange_;
    float viewHalfAngleRad_;

    VECTOR forwardDir_;
};