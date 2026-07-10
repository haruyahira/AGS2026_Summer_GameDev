#include <DxLib.h>
#include <math.h>
#include <memory>
#include <string>

#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../Common/AnimationController.h"
#include "../Player.h"
#include "../../Application.h"
#include "EnemyPolice.h"

EnemyPolice::EnemyPolice(void) : EnemyBase()
{
    speed_ = 350.0f;

    waitTimer_ = 0.0f;
    isWaiting_ = false;

    animType_ = ANIM_TYPE::IDLE;

    radius_ = 5.0f;

    targetIndex_ = 0;

    // プレイヤーと同じ高さ付近に合わせる
    groundY_ = -90.0f;

    isChasing_ = false;

    // 視野距離
    viewRange_ = 600.0f;

    // 視野角：左右45度、合計90度
    viewHalfAngleRad_ = AsoUtility::Deg2RadF(45.0f);

    forwardDir_ = VGet(0.0f, 0.0f, 1.0f);
    waitBaseDir_ = VGet(0.0f, 0.0f, 1.0f);

    // 攻撃設定
    attackRange_ = 120.0f;       // 攻撃開始距離
    attackRadius_ = 45.0f;       // 攻撃判定半径


    attackDuration_ = 0.6f;      // プレイヤーと同じ
    attackInterval_ = 0.7f;      // 次の攻撃まで

    attackIntervalTimer_ = 0.0f;

    isAttacking_ = false;
    isAttackHit_ = false;

    rightHandFrame_ = -1;
    leftHandFrame_ = -1;

    isRightAttack_ = true;

    bodyHeight_ = 130.0f;
    bodyCenterOffsetY_ = 65.0f;

    maxHp_ = 168;
    attackPower_ = 2;
}

EnemyPolice::~EnemyPolice(void)
{

}

void EnemyPolice::Init(void)
{
    // モデルの基本設定
    transform_.SetModel(resMng_.LoadModelDuplicate(
        ResourceManager::SRC::ENEMYPOLICE));

    MV1SetAmbColorScale(
        transform_.modelId,
        GetColorF(1.0f, 1.0f, 1.0f, 1.0f)
    );


    transform_.scl = { 0.1f, 0.1f, 0.1f };
    transform_.pos = { -100.0f, -30.0f, 0.0f };

    transform_.quaRot = Quaternion();

    transform_.quaRotLocal =
        Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });

    transform_.Update();

    InitAnimation();
    InitAttackFrame();

    InitHP(maxHp_, 10.0f);

    // 追加：敵の3D足音
    InitFootstep3DSound();


    PlayAnimation(ANIM_TYPE::IDLE, true);

    // 最初の目的地を決める
    DecideNextTarget();
}

void EnemyPolice::Draw(void)
{
    // モデル、当たり判定、視野、攻撃判定はBaseで描画
    EnemyBase::Draw();


#ifdef _DEBUG
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetUseLighting(FALSE);
    SetUseBackCulling(FALSE);

    // 敵から現在の目的地まで線を引く
    if (!patrolPoints_.empty() &&
        targetIndex_ >= 0 &&
        targetIndex_ < static_cast<int>(patrolPoints_.size()))
    {
        VECTOR targetDrawPos = patrolPoints_[targetIndex_];
        targetDrawPos.y += 30.0f;

        DrawLine3D(
            transform_.pos,
            targetDrawPos,
            GetColor(255, 255, 0));
    }
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
    SetUseBackCulling(TRUE);

#endif
#ifdef _DEBUG
    DrawFormatString(
        20,
        120,
        GetColor(255, 255, 0),
        "Enemy Anim:%d  Attack:%d  Timer:%.2f  AnimEnd:%d",
        currentAnimType_,
        isAttacking_,
        attackTimer_,
        animationController_ ? animationController_->IsEnd() : -1
    );
#endif
}

void EnemyPolice::SetPatrolPoints(const std::vector<VECTOR>& points)
{
    patrolPoints_ = points;

    if (!patrolPoints_.empty())
    {
        targetIndex_ = 0;

        transform_.pos = patrolPoints_[0];
        transform_.pos.y = groundY_;
    }
}

void EnemyPolice::SetPatrolLinks(const std::vector<std::vector<int>>& links)
{
    patrolLinks_ = links;
}

void EnemyPolice::UpdateWander(Player* player)
{
    if (patrolPoints_.empty())
    {
        animType_ = ANIM_TYPE::IDLE;
        ChangeAnimation(static_cast<int>(animType_));
        return;
    }

    // 待機中
    if (isWaiting_)
    {
        animType_ = ANIM_TYPE::IDLE;
        ChangeAnimation(static_cast<int>(animType_));

        waitTimer_ -= 1.0f;

        // 首振り処理
        float angle =
            sinf(waitTimer_ * 0.03f) *
            AsoUtility::Deg2RadF(60.0f);

        VECTOR base = waitBaseDir_;

        float baseAngle = atan2f(base.x, base.z);
        float lookAngle = baseAngle + angle;

        forwardDir_ = VGet(
            sinf(lookAngle),
            0.0f,
            cosf(lookAngle));

        transform_.quaRot =
            Quaternion::AngleAxis(
                lookAngle,
                AsoUtility::AXIS_Y);

        if (waitTimer_ <= 0.0f)
        {
            isWaiting_ = false;
            DecideNextTarget();
        }

        return;
    }

    animType_ = ANIM_TYPE::RUN;
    ChangeAnimation(static_cast<int>(animType_));

    if (targetIndex_ < 0 ||
        targetIndex_ >= static_cast<int>(patrolPoints_.size()))
    {
        targetIndex_ = 0;
    }

    VECTOR target = patrolPoints_[targetIndex_];

    VECTOR toTarget = VSub(target, transform_.pos);
    toTarget.y = 0.0f;

    float dist = VSize(toTarget);

    // 到着
    if (dist < 10.0f)
    {
        isWaiting_ = true;
        waitTimer_ = 300.0f;
        waitBaseDir_ = forwardDir_;
        return;
    }

    VECTOR dir = VNorm(toTarget);

    forwardDir_ = dir;

    VECTOR beforePos = transform_.pos;


    float dt =
        SceneManager::GetInstance().GetDeltaTime();

    transform_.pos = VAdd(
        transform_.pos,
        VScale(dir, speed_ * dt));

    transform_.pos.y = groundY_;

    // 壁や机に当たったら戻して別の目的地へ
    if (CollisionFurniture(player, beforePos))
    {
        transform_.pos = beforePos;
        DecideNextTarget();
        return;
    }

    float angleY = atan2f(dir.x, dir.z);

    transform_.quaRot =
        Quaternion::AngleAxis(angleY, AsoUtility::AXIS_Y);
}

void EnemyPolice::UpdateChase(Player* player)
{
    if (player == nullptr)
    {
        PlayAnimation(ANIM_TYPE::IDLE, true);
        return;
    }

    VECTOR playerPos = player->GetTransform().pos;

    VECTOR toPlayer = VSub(playerPos, transform_.pos);
    toPlayer.y = 0.0f;

    float dist = VSize(toPlayer);

    if (dist < 5.0f)
    {
        PlayAnimation(ANIM_TYPE::IDLE, true);
        return;
    }

    PlayAnimation(ANIM_TYPE::RUN, true);

    VECTOR dir = VNorm(toPlayer);

    VECTOR beforePos = transform_.pos;

    float dt =
        SceneManager::GetInstance().GetDeltaTime();

    float chaseSpeed = speed_ * 1.5f;

    transform_.pos = VAdd(
        transform_.pos,
        VScale(dir, chaseSpeed * dt));

    transform_.pos.y = groundY_;

    if (CollisionFurniture(player, beforePos))
    {
        transform_.pos = beforePos;
        return;
    }

    forwardDir_ = dir;

    float angleY = atan2f(dir.x, dir.z);

    transform_.quaRot =
        Quaternion::AngleAxis(angleY, AsoUtility::AXIS_Y);
}

void EnemyPolice::StartAttack(Player* player)
{
    if (player == nullptr)
    {
        return;
    }

    isAttacking_ = true;
    isAttackHit_ = false;

    attackTimer_ = attackDuration_;

    // プレイヤーの方向を向く
    VECTOR toPlayer = VSub(player->GetTransform().pos, transform_.pos);
    toPlayer.y = 0.0f;

    if (VSize(toPlayer) > 0.001f)
    {
        VECTOR dir = VNorm(toPlayer);
        forwardDir_ = dir;

        float angleY = atan2f(dir.x, dir.z);

        transform_.quaRot =
            Quaternion::AngleAxis(angleY, AsoUtility::AXIS_Y);
    }

    // プレイヤーと同じ方式で攻撃アニメーション再生
    if (isRightAttack_)
    {
        animType_ = ANIM_TYPE::HIT_R;
        currentAnimType_ = static_cast<int>(ANIM_TYPE::HIT_R);

        animationController_->Play(
            static_cast<int>(ANIM_TYPE::HIT_R),
            false);
    }
    else
    {
        animType_ = ANIM_TYPE::HIT_L;
        currentAnimType_ = static_cast<int>(ANIM_TYPE::HIT_L);

        animationController_->Play(
            static_cast<int>(ANIM_TYPE::HIT_L),
            false);
    }

    // プレイヤーと同じく、攻撃開始時に次の手へ切り替える
    isRightAttack_ = !isRightAttack_;
}
void EnemyPolice::UpdateAttack(Player* player)
{
    attackTimer_ -= SceneManager::GetInstance().GetDeltaTime();

    // 攻撃開始から何秒経ったか
    float elapsed = attackDuration_ - attackTimer_;

    // 攻撃判定タイミング
    if (elapsed >= 0.20f &&
        elapsed <= 0.40f &&
        !isAttackHit_)
    {
        if (IsEnemyAttackHitPlayer(player))
        {
            isAttackHit_ = true;

            if (player != nullptr)
            {
                player->Damage(attackPower_);
            }
        }
    }

    // プレイヤーと同じく、タイマーで攻撃終了
    if (attackTimer_ <= 0.0f)
    {
        isAttacking_ = false;
        isAttackHit_ = false;

        attackIntervalTimer_ = attackInterval_;

        currentAnimType_ = -1;
        PlayAnimation(ANIM_TYPE::IDLE, true);
    }
}
VECTOR EnemyPolice::GetAttackPos(void) const
{
    // 右パンチ中
    if (animType_ == ANIM_TYPE::HIT_R)
    {
        if (rightHandFrame_ != -1)
        {
            return MV1GetFramePosition(
                transform_.modelId,
                rightHandFrame_);
        }
    }

    // 左パンチ中
    if (animType_ == ANIM_TYPE::HIT_L)
    {
        if (leftHandFrame_ != -1)
        {
            return MV1GetFramePosition(
                transform_.modelId,
                leftHandFrame_);
        }
    }

    return transform_.pos;
}

void EnemyPolice::DecideNextTarget(void)
{
    if (patrolPoints_.empty())
    {
        return;
    }

    // リンク情報が無いならランダム
    if (patrolLinks_.empty())
    {
        int next = targetIndex_;

        while (next == targetIndex_ && patrolPoints_.size() > 1)
        {
            next = GetRand(static_cast<int>(patrolPoints_.size()) - 1);
        }

        targetIndex_ = next;
        return;
    }

    if (targetIndex_ < 0 ||
        targetIndex_ >= static_cast<int>(patrolLinks_.size()))
    {
        targetIndex_ = 0;
        return;
    }

    const auto& nextList = patrolLinks_[targetIndex_];

    if (nextList.empty())
    {
        return;
    }

    int randIndex = GetRand(static_cast<int>(nextList.size()) - 1);
    int nextTarget = nextList[randIndex];

    if (nextTarget < 0 ||
        nextTarget >= static_cast<int>(patrolPoints_.size()))
    {
        return;
    }

    targetIndex_ = nextTarget;
}

void EnemyPolice::InitAnimation(void)
{
    std::string path = Application::PATH_MODEL + "Enemy/";

    animationController_ =
        std::make_unique<AnimationController>(transform_.modelId);

    // 本当は敵用Idleが理想
    animationController_->Add(
        static_cast<int>(ANIM_TYPE::IDLE),
        path + "Idle.mv1",
        0.0f);

    animationController_->Add(
        static_cast<int>(ANIM_TYPE::RUN),
        path + "Walk.mv1",
        30.0f);

    // 右攻撃
    animationController_->Add(
        static_cast<int>(ANIM_TYPE::HIT_R),
        Application::PATH_MODEL + "Player/Hit_R.mv1",
        30.0f);

    // 左攻撃
    animationController_->Add(
        static_cast<int>(ANIM_TYPE::HIT_L),
        Application::PATH_MODEL + "Player/Hit_L.mv1",
        30.0f);
}

void EnemyPolice::InitAttackFrame(void)
{
    rightHandFrame_ = MV1SearchFrame(transform_.modelId, "Hand.R");
    leftHandFrame_ = MV1SearchFrame(transform_.modelId, "Hand.L");

    // モデルによってボーン名が違う場合の保険
    if (rightHandFrame_ == -1)
    {
        rightHandFrame_ = MV1SearchFrame(transform_.modelId, "RightHand");
    }

    if (leftHandFrame_ == -1)
    {
        leftHandFrame_ = MV1SearchFrame(transform_.modelId, "LeftHand");
    }
}

int EnemyPolice::GetAnimType(void) const
{
    return static_cast<int>(animType_);
}
void EnemyPolice::SetPos(const VECTOR& pos)
{
    transform_.pos = pos;
    transform_.Update();
}

void EnemyPolice::PlayAnimation(ANIM_TYPE animType, bool isLoop)
{
    if (animationController_ == nullptr)
    {
        return;
    }

    int nextAnimType = static_cast<int>(animType);

    // 同じアニメーションなら再生し直さない
    if (currentAnimType_ == nextAnimType)
    {
        return;
    }

    this->animType_ = animType;
    currentAnimType_ = nextAnimType;

    animationController_->Play(nextAnimType, isLoop);
}