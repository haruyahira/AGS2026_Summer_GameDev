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
#include "EnemyNormal.h"

EnemyNormal::EnemyNormal(void) : EnemyBase()
{
    speed_ = 300.0f;

    waitTimer_ = 0.0f;
    isWaiting_ = false;

    animType_ = ANIM_TYPE::IDLE;

    radius_ = 23.0f;

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
    attackRange_ = 95.0f;
    attackRadius_ = 40.0f;
    attackDuration_ = 45.0f;
    attackInterval_ = 90.0f;
    attackIntervalTimer_ = 0.0f;

    isAttacking_ = false;
    isAttackHit_ = false;

    rightHandFrame_ = -1;
    leftHandFrame_ = -1;

    isRightAttack_ = true;

    bodyHeight_ = 130.0f;
    bodyCenterOffsetY_ = 65.0f;

    maxHp_ = 168;
    attackPower_ = 1;
}

EnemyNormal::~EnemyNormal(void)
{
}

void EnemyNormal::Init(void)
{
    // モデルの基本設定
    transform_.SetModel(resMng_.LoadModelDuplicate(
        ResourceManager::SRC::ENEMYNORMAL));

    transform_.scl = { 0.1f, 0.1f, 0.1f };
    transform_.pos = { -100.0f, -30.0f, 0.0f };

    transform_.quaRot = Quaternion();

    transform_.quaRotLocal =
        Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });

    transform_.Update();

    InitAnimation();
    InitAttackFrame();

    InitHP(maxHp_, 10.0f);

    ChangeAnimation(static_cast<int>(ANIM_TYPE::IDLE));

    // 最初の目的地を決める
    DecideNextTarget();
}

void EnemyNormal::Draw(void)
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
}

void EnemyNormal::SetPatrolPoints(const std::vector<VECTOR>& points)
{
    patrolPoints_ = points;

    if (!patrolPoints_.empty())
    {
        targetIndex_ = 0;

        transform_.pos = patrolPoints_[0];
        transform_.pos.y = groundY_;
    }
}

void EnemyNormal::SetPatrolLinks(const std::vector<std::vector<int>>& links)
{
    patrolLinks_ = links;
}

void EnemyNormal::UpdateWander(Player* player)
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

void EnemyNormal::UpdateChase(Player* player)
{
    if (player == nullptr)
    {
        animType_ = ANIM_TYPE::IDLE;
        ChangeAnimation(static_cast<int>(animType_));
        return;
    }

    animType_ = ANIM_TYPE::RUN;
    ChangeAnimation(static_cast<int>(animType_));

    VECTOR playerPos = player->GetTransform().pos;

    VECTOR toPlayer = VSub(playerPos, transform_.pos);
    toPlayer.y = 0.0f;

    float dist = VSize(toPlayer);

    if (dist < 5.0f)
    {
        return;
    }

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

void EnemyNormal::StartAttack(Player* player)
{
    if (player == nullptr)
    {
        return;
    }

    isAttacking_ = true;
    isAttackHit_ = false;

    attackTimer_ = attackDuration_;

    animType_ = ANIM_TYPE::ATTACK;

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

    // 攻撃アニメーションはループしない
    ChangeAnimation(static_cast<int>(ANIM_TYPE::ATTACK), false);
}

void EnemyNormal::UpdateAttack(Player* player)
{
    animType_ = ANIM_TYPE::ATTACK;

    attackTimer_ -= 1.0f;

    // 攻撃判定タイミング
    // 45F中、中盤だけ当たる
    if (attackTimer_ <= 30.0f &&
        attackTimer_ >= 15.0f &&
        !isAttackHit_)
    {
        if (IsEnemyAttackHitPlayer(player))
        {
            isAttackHit_ = true;


#ifdef _DEBUG
           // printfDx("Enemy Attack Hit Player\n");
#endif

            player->Damage(attackPower_);

        }
    }

    // 攻撃終了
    if (attackTimer_ <= 0.0f)
    {
        isAttacking_ = false;
        isAttackHit_ = false;

        attackIntervalTimer_ = attackInterval_;

        animType_ = ANIM_TYPE::IDLE;
        ChangeAnimation(static_cast<int>(animType_));

        // 次回攻撃の手を反対にする
        isRightAttack_ = !isRightAttack_;
    }
}

VECTOR EnemyNormal::GetAttackPos(void) const
{
    if (isRightAttack_)
    {
        if (rightHandFrame_ != -1)
        {
            return MV1GetFramePosition(
                transform_.modelId,
                rightHandFrame_);
        }
    }
    else
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

void EnemyNormal::DecideNextTarget(void)
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

void EnemyNormal::InitAnimation(void)
{
    std::string path = Application::PATH_MODEL + "Enemy/";

    animationController_ =
        std::make_unique<AnimationController>(transform_.modelId);

    animationController_->Add(
        static_cast<int>(ANIM_TYPE::IDLE),
        "Player/Hit.mv1",
        0.0f);

    animationController_->Add(
        static_cast<int>(ANIM_TYPE::RUN),
        path + "Run.mv1",
        30.0f);

    // 敵用Attack.mv1があるならこれ
    animationController_->Add(
        static_cast<int>(ANIM_TYPE::ATTACK),
        path + "Attack.mv1",
        30.0f);

    // Attack.mv1がまだ無い場合は、上をコメントアウトして一時的にこれでも可
    /*
    animationController_->Add(
        static_cast<int>(ANIM_TYPE::ATTACK),
        "Player/Hit_R.mv1",
        30.0f);
    */
}

void EnemyNormal::InitAttackFrame(void)
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

int EnemyNormal::GetAnimType(void) const
{
    return static_cast<int>(animType_);
}
void EnemyNormal::SetPos(const VECTOR& pos)
{
    transform_.pos = pos;
    transform_.Update();
}
