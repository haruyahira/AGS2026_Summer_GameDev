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

    // Playerを受け取る敵用Update
    virtual void Update(Player* player);

    // 共通描画
    virtual void Draw(void) override;

    bool IsDead(void) const;

    VECTOR GetPos(void) const;


protected:
    // 敵ごとの個性
    virtual void UpdateWander(Player* player) = 0;
    virtual void UpdateChase(Player* player) = 0;
    virtual void StartAttack(Player* player) = 0;
    virtual void UpdateAttack(Player* player) = 0;
    virtual void InitAnimation(void) = 0;

    // 派生クラス側の現在アニメーション番号
    virtual int GetAnimType(void) const = 0;

    // 攻撃判定の中心位置
    virtual VECTOR GetAttackPos(void) const = 0;

    // 共通処理
    bool CheckPlayerAttack(Player* player);
    bool IsHitPlayer(Player* player);
    bool CollisionFurniture(Player* player, VECTOR beforePos);

    bool IsPlayerInView(Player* player);
    bool IsBlockedByWall(Player* player);

    bool IsPlayerInAttackRange(Player* player);
    bool IsEnemyAttackHitPlayer(Player* player);

    void DrawViewRange(void);
    void DrawAttackRange(void);

    // アニメーション変更
    void ChangeAnimation(int animType, bool isLoop = true);


    void InitHP(int maxHp, float invincibleFrame = 20.0f);
    void Damage(int damage);
  

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

    // 敵攻撃用
    bool isAttacking_;
    bool isAttackHit_;

    float attackTimer_;
    float attackDuration_;

    float attackIntervalTimer_;
    float attackInterval_;

    float attackRange_;
    float attackRadius_;

    int currentAnimType_;


    int maxHp_;
    int attackPower_;


    float bodyHeight_;
    float bodyCenterOffsetY_;


};