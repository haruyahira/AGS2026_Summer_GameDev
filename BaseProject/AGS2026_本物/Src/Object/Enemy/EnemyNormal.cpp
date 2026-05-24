#include <DxLib.h>
#include <math.h>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../Furniture/Furniture.h"
#include "../Player.h"
#include "EnemyNormal.h"

EnemyNormal::EnemyNormal(void) : EnemyBase()
{
    speed_ = 1.2f;
    waitTimer_ = 0.0f;
    isWaiting_ = false;

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

    // 徘徊開始地点
    /*startPos_ = transform_.pos;*/

    // 最初の目的地を決める
    DecideNextTarget();
}

void EnemyNormal::Update(Player* player)
{

    // 視野にプレイヤーがいるかチェック
    if (IsPlayerInView(player))
    {
        isChasing_ = true;
    }
    else
    {
        // 視野から出たら追跡終了
        isChasing_ = false;
    }

    if (isChasing_)
    {
        UpdateChase(player);
    }
    else
    {
        UpdateWander(player);
    }

    // 床高さに固定
    transform_.pos.y = groundY_;

    // プレイヤーとの当たり判定
    if (IsHitPlayer(player))
    {
        printfDx("Enemy Hit Player\n");
    }

    transform_.Update();

}

void EnemyNormal::Update(void)
{
    Update(nullptr);
}

void EnemyNormal::Draw(void)
{
    // モデルの描画
    MV1DrawModel(transform_.modelId);

#ifdef _DEBUG
    // 当たり判定表示
    DrawSphere3D(
        transform_.pos,
        radius_,
        16,
        GetColor(255, 0, 0),
        GetColor(255, 0, 0),
        FALSE);


    // 巡回ポイント表示
    for (int i = 0; i < static_cast<int>(patrolPoints_.size()); i++)
    {
        unsigned int color = GetColor(0, 100, 255);

        // 現在の目的地だけ黄色
        if (i == targetIndex_)
        {
            color = GetColor(255, 255, 0);
        }

        // 表示用に少し上へずらす
        VECTOR drawPos = patrolPoints_[i];
        drawPos.y += 30.0f;

        // 巡回ポイントを球で表示
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
            VECTOR p1 = patrolPoints_[i];
            VECTOR p2 = patrolPoints_[next];

            p1.y += 30.0f;
            p2.y += 30.0f;

            DrawLine3D(
                p1,
                p2,
                GetColor(0, 100, 255));
        }


        // 敵から現在の目的地まで線を引く
        if (!patrolPoints_.empty())
        {
            VECTOR targetDrawPos = patrolPoints_[targetIndex_];
            targetDrawPos.y += 30.0f;

            DrawLine3D(
                transform_.pos,
                targetDrawPos,
                GetColor(255, 255, 0));
        }

    }

    // 視野表示
    DrawViewRange();


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

void EnemyNormal::UpdateWander(Player* player)
{

    if (patrolPoints_.empty())
    {
        return;
    }


    // 待機中
    if (isWaiting_)
    {
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
            cosf(lookAngle)
        );

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

    VECTOR target = patrolPoints_[targetIndex_];

    VECTOR toTarget = VSub(target, transform_.pos);
    toTarget.y = 0.0f;

    float dist = VSize(toTarget);

    // 到着
    if (dist < 10.0f)
    {

        isWaiting_ = true;

        // 待機時間を少し長くする
        waitTimer_ = 120.0f;

        // 首振りの基準方向を保存
        waitBaseDir_ = forwardDir_;

        return;

    }

    VECTOR dir = VNorm(toTarget);
    // 方向を更新
    forwardDir_ = dir;

    VECTOR beforePos = transform_.pos;

    transform_.pos = VAdd(
        transform_.pos,
        VScale(dir, speed_)
    );

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

void EnemyNormal::DecideNextTarget()
{
    if (patrolPoints_.empty())
    {
        return;
    }

    // リンク情報が無いなら今まで通り
    if (patrolLinks_.empty())
    {
        int next = targetIndex_;

        while (next == targetIndex_ && patrolPoints_.size() > 1)
        {
            next = GetRand((int)patrolPoints_.size() - 1);
        }

        targetIndex_ = next;
        return;
    }

    // 現在ポイントから行けるポイント一覧
    const auto& nextList = patrolLinks_[targetIndex_];

    if (nextList.empty())
    {
        return;
    }

    int randIndex = GetRand((int)nextList.size() - 1);

    targetIndex_ = nextList[randIndex];
}
bool EnemyNormal::IsHitPlayer(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    VECTOR playerPos = player->GetTransform().pos;

    VECTOR diff = VSub(playerPos, transform_.pos);
    diff.y = 0.0f;

    float distSq = VDot(diff, diff);

    float playerRadius = 15.0f;
    float hitRange = radius_ + playerRadius;

    return distSq <= hitRange * hitRange;
}
bool EnemyNormal::CollisionFurniture(Player* player, VECTOR beforePos)
{

    if (player == nullptr)
    {
        return false;
    }

    const auto& furnitures = player->GetFurnitures();

    float bottomY = transform_.pos.y;
    float topY = transform_.pos.y + 80.0f;

    for (auto f : furnitures)
    {
        if (f == nullptr) continue;

        // Wall / Ceiling などOBB系
        if (f->ResolveCollision(
            transform_.pos,
            radius_,
            bottomY,
            topY))
        {
            return true;
        }

        // Table / Showcase などBoxCollider系
        for (const auto& box : f->GetColliders())
        {
            float boxBottom = box.center.y - box.halfSize.y;
            float boxTop = box.center.y + box.halfSize.y;

            if (bottomY > boxTop || topY < boxBottom)
            {
                continue;
            }

            float minX = box.center.x - box.halfSize.x;
            float maxX = box.center.x + box.halfSize.x;
            float minZ = box.center.z - box.halfSize.z;
            float maxZ = box.center.z + box.halfSize.z;

            float closestX = fmaxf(minX, fminf(transform_.pos.x, maxX));
            float closestZ = fmaxf(minZ, fminf(transform_.pos.z, maxZ));

            float diffX = transform_.pos.x - closestX;
            float diffZ = transform_.pos.z - closestZ;

            float distSq = diffX * diffX + diffZ * diffZ;

            if (distSq < radius_ * radius_)
            {
                return true;
            }
        }
    }

    return false;

}

void EnemyNormal::SetPatrolLinks(const std::vector<std::vector<int>>& links)
{
    patrolLinks_ = links;
}

bool EnemyNormal::IsPlayerInView(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    VECTOR playerPos = player->GetTransform().pos;

    VECTOR toPlayer = VSub(playerPos, transform_.pos);

    // XZ平面だけで見る
    toPlayer.y = 0.0f;

    float dist = VSize(toPlayer);

    // 距離外なら見えていない
    if (dist > viewRange_)
    {
        return false;
    }

    if (dist < 0.001f)
    {
        return true;
    }

    VECTOR dirToPlayer = VNorm(toPlayer);

    VECTOR forward = forwardDir_;
    forward.y = 0.0f;
    forward = VNorm(forward);

    // 内積で角度判定
    float dot = VDot(forward, dirToPlayer);

    float limit = cosf(viewHalfAngleRad_);


    // 視野角外なら見えていない
    if (dot < limit)
    {
        return false;
    }

    // 壁で遮られていたら見えていない
    if (IsBlockedByWall(player))
    {
        return false;
    }

    return true;

}

void EnemyNormal::DrawViewRange()
{
#ifdef _DEBUG

    VECTOR basePos = transform_.pos;
    //basePos.y += 50.0f;

    VECTOR forward = forwardDir_;
    forward.y = 0.0f;

    if (VSize(forward) < 0.001f)
    {
        forward = VGet(0.0f, 0.0f, 1.0f);
    }

    forward = VNorm(forward);

    float baseAngle = atan2f(forward.x, forward.z);

    float leftAngle = baseAngle + viewHalfAngleRad_;
    float rightAngle = baseAngle - viewHalfAngleRad_;

    VECTOR leftDir = VGet(
        sinf(leftAngle),
        0.0f,
        cosf(leftAngle)
    );

    VECTOR rightDir = VGet(
        sinf(rightAngle),
        0.0f,
        cosf(rightAngle)
    );

    VECTOR leftEnd = VAdd(
        basePos,
        VScale(leftDir, viewRange_)
    );

    VECTOR rightEnd = VAdd(
        basePos,
        VScale(rightDir, viewRange_)
    );

    unsigned int color = isChasing_
        ? GetColor(255, 0, 0)
        : GetColor(0, 255, 0);

    // 視野の左右線
    DrawLine3D(basePos, leftEnd, color);
    DrawLine3D(basePos, rightEnd, color);

    // 視野の外周をざっくり円弧で描く
    const int DIV = 16;

    VECTOR prev = leftEnd;

    for (int i = 1; i <= DIV; i++)
    {
        float t = static_cast<float>(i) / DIV;

        float angle =
            leftAngle + (rightAngle - leftAngle) * t;

        VECTOR dir = VGet(
            sinf(angle),
            0.0f,
            cosf(angle)
        );

        VECTOR p = VAdd(
            basePos,
            VScale(dir, viewRange_)
        );

        DrawLine3D(prev, p, color);

        prev = p;
    }

#endif
}

void EnemyNormal::UpdateChase(Player* player)
{
    if (player == nullptr)
    {
        return;
    }

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
        VScale(dir, chaseSpeed)
    );

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


bool EnemyNormal::IsBlockedByWall(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    VECTOR start = transform_.pos;
    VECTOR end = player->GetTransform().pos;

    // 視線の高さを少し上げる
    start.y += 60.0f;
    end.y += 60.0f;

    const auto& furnitures = player->GetFurnitures();

    for (auto f : furnitures)
    {
        if (f == nullptr)
        {
            continue;
        }

        // 壁などが視線を遮っているか
        if (f->IsBlockingSight(start, end))
        {
            return true;
        }
    }

    return false;
}
