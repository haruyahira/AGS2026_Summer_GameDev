#include <DxLib.h>
#include <math.h>
#include <memory>
#include <string>

#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/AnimationController.h"
#include "../Player.h"
#include "../../Application.h"
#include "EnemyNormal.h"

EnemyNormal::EnemyNormal(void) : EnemyBase()
{
    speed_ = 1.2f;

    waitTimer_ = 0.0f;
    isWaiting_ = false;

    animType_ = ANIM_TYPE::IDLE;

    radius_ = 23.0f;

    targetIndex_ = 0;

    // プレイヤーと同じ高さ付近に合わせる
    groundY_ = -30.0f;

    // 追跡状態
    isChasing_ = false;

    // 視野距離
    viewRange_ = 600.0f;

    // 視野角：左右45度、合計90度
    viewHalfAngleRad_ = AsoUtility::Deg2RadF(45.0f);

    // 初期正面方向
    forwardDir_ = VGet(0.0f, 0.0f, 1.0f);

    waitBaseDir_ = VGet(0.0f, 0.0f, 1.0f);
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

    // 最初の目的地を決める
    DecideNextTarget();
}

void EnemyNormal::Draw(void)
{
    // モデル、当たり判定、視野はBaseで描画
    EnemyBase::Draw();

#ifdef _DEBUG

    // 巡回ポイント表示
    for (int i = 0; i < static_cast<int>(patrolPoints_.size()); i++)
    {
        unsigned int color = GetColor(0, 100, 255);

        // 現在の目的地だけ黄色
        if (i == targetIndex_)
        {
            color = GetColor(255, 255, 0);
        }

        VECTOR drawPos = patrolPoints_[i];
        drawPos.y += 30.0f;

        DrawSphere3D(
            drawPos,
            20.0f,
            12,
            color,
            color,
            FALSE);
    }

    // 巡回ポイント同士を線でつなぐ
    for (int i = 0; i < static_cast<int>(patrolLinks_.size()); i++)
    {
        for (int next : patrolLinks_[i])
        {
            if (next < 0 || next >= static_cast<int>(patrolPoints_.size()))
            {
                continue;
            }

            VECTOR p1 = patrolPoints_[i];
            VECTOR p2 = patrolPoints_[next];

            p1.y += 30.0f;
            p2.y += 30.0f;

            DrawLine3D(
                p1,
                p2,
                GetColor(0, 100, 255));
        }
    }

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
        return;
    }

    // 待機中
    if (isWaiting_)
    {
        animType_ = ANIM_TYPE::IDLE;

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

        // 見た目の向きも首振り方向へ向ける
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

        // 待機時間
        waitTimer_ = 300.0f;

        // 首振りの基準方向を保存
        waitBaseDir_ = forwardDir_;

        return;
    }

    VECTOR dir = VNorm(toTarget);

    // 正面方向を更新
    forwardDir_ = dir;

    VECTOR beforePos = transform_.pos;

    transform_.pos = VAdd(
        transform_.pos,
        VScale(dir, speed_));

    transform_.pos.y = groundY_;

    // 壁や机に当たったら戻して別の目的地へ
    if (CollisionFurniture(player, beforePos))
    {
        transform_.pos = beforePos;
        DecideNextTarget();
        return;
    }

    // 進行方向を向く
    float angleY = atan2f(dir.x, dir.z);

    transform_.quaRot =
        Quaternion::AngleAxis(angleY, AsoUtility::AXIS_Y);
}

void EnemyNormal::UpdateChase(Player* player)
{
    if (player == nullptr)
    {
        animType_ = ANIM_TYPE::IDLE;
        return;
    }

    animType_ = ANIM_TYPE::RUN;

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

    float chaseSpeed = speed_ * 1.5f;

    transform_.pos = VAdd(
        transform_.pos,
        VScale(dir, chaseSpeed));

    transform_.pos.y = groundY_;

    // 壁や机に当たったら戻す
    if (CollisionFurniture(player, beforePos))
    {
        transform_.pos = beforePos;
        return;
    }

    // 正面方向を更新
    forwardDir_ = dir;

    // 向きを進行方向へ
    float angleY = atan2f(dir.x, dir.z);

    transform_.quaRot =
        Quaternion::AngleAxis(angleY, AsoUtility::AXIS_Y);
}

void EnemyNormal::DecideNextTarget(void)
{
    if (patrolPoints_.empty())
    {
        return;
    }

    // リンク情報が無いならランダムに目的地を選ぶ
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

    // targetIndex_ がリンク配列の範囲外なら補正
    if (targetIndex_ < 0 ||
        targetIndex_ >= static_cast<int>(patrolLinks_.size()))
    {
        targetIndex_ = 0;
        return;
    }

    // 現在ポイントから行けるポイント一覧
    const auto& nextList = patrolLinks_[targetIndex_];

    if (nextList.empty())
    {
        return;
    }

    int randIndex = GetRand(static_cast<int>(nextList.size()) - 1);

    int nextTarget = nextList[randIndex];

    // 不正なインデックスなら無視
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
}

int EnemyNormal::GetAnimType(void) const
{
    return static_cast<int>(animType_);
}