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
    int GetHP(void) const;

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
    // 足音反応
    bool isHearingFootstep_;
    float hearingTimer_;
    float hearingDuration_;
    VECTOR lastHeardPos_;

    // 足音検知範囲
    float footstepHearRangeNormal_;
    float footstepHearRangeAlert_;

    float GetCurrentFootstepHearRange(void) const;

    // 警戒状態
    bool isAlert_;
    float alertTimer_;
    float alertDuration_;
    float alertFootstepRangeRate_;

    bool CanHearPlayerFootstep(Player* player);
    void StartHearFootstep(Player* player);
    void UpdateHearFootstep(Player* player);

    void StartAlert(void);
    void UpdateAlert(Player* player);

    void LookAtPosition(const VECTOR& targetPos);
    
#ifdef _DEBUG
    float debugFootstepHearRange_;
    bool debugCanHearFootstep_;
    void DrawFootstepHearRangeDebug(void);
#endif

    bool wasChasing_;
    VECTOR lastKnownPlayerPos_;

    bool isPlayerAttackHit_;
};