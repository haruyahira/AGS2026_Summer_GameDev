#include <DxLib.h>
#include <math.h>

#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "../Furniture/Furniture.h"
#include "../Player.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(void) : ActorBase()
{
    speed_ = 1.0f;
    radius_ = 20.0f;
    groundY_ = -30.0f;

    isDead_ = false;
    isChasing_ = false;

    viewRange_ = 500.0f;
    viewHalfAngleRad_ = AsoUtility::Deg2RadF(45.0f);

    forwardDir_ = VGet(0.0f, 0.0f, 1.0f);
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Update(void)
{
    Update(nullptr);
}

void EnemyBase::Update(Player* player)
{
    if (isDead_)
    {
        return;
    }

    // プレイヤーの攻撃判定
    if (CheckPlayerAttack(player))
    {
        isDead_ = true;
        printfDx("Enemy Dead\n");
        return;
    }

    // 視野判定
    if (IsPlayerInView(player))
    {
        isChasing_ = true;
    }
    else
    {
        isChasing_ = false;
    }

    // 状態に応じて行動
    if (isChasing_)
    {
        UpdateChase(player);
    }
    else
    {
        UpdateWander(player);
    }

    // アニメーション更新
    if (animationController_)
    {
        animationController_->Update();
        animationController_->Play(GetAnimType());
    }

    // 床高さ固定
    transform_.pos.y = groundY_;

    // プレイヤーとの接触判定
    if (IsHitPlayer(player))
    {
        printfDx("Enemy Hit Player\n");
    }

    transform_.Update();
}

void EnemyBase::Draw(void)
{
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

    // 視野表示
    DrawViewRange();

#endif
}

bool EnemyBase::CheckPlayerAttack(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    if (!player->IsAttacking())
    {
        return false;
    }

    VECTOR attackPos = player->GetAttackPos();

    VECTOR toEnemy = VSub(transform_.pos, attackPos);
    toEnemy.y = 0.0f;

    float dist = VSize(toEnemy);

    float attackRadius = 45.0f;

    if (dist > attackRadius)
    {
        return false;
    }

    return true;
}

bool EnemyBase::IsHitPlayer(Player* player)
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

bool EnemyBase::CollisionFurniture(Player* player, VECTOR beforePos)
{
    // 現状 beforePos は呼び出し側で戻すために使う
    (void)beforePos;

    if (player == nullptr)
    {
        return false;
    }

    const auto& furnitures = player->GetFurnitures();

    float bottomY = transform_.pos.y;
    float topY = transform_.pos.y + 80.0f;

    for (auto f : furnitures)
    {
        if (f == nullptr)
        {
            continue;
        }

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

bool EnemyBase::IsPlayerInView(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    // 机の下に隠れているなら見えない
    if (player->IsHiddenUnderFurniture())
    {
        return false;
    }

    VECTOR playerPos = player->GetTransform().pos;

    VECTOR toPlayer = VSub(playerPos, transform_.pos);
    toPlayer.y = 0.0f;

    float dist = VSize(toPlayer);

    // 視野距離外
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

    if (VSize(forward) < 0.001f)
    {
        forward = VGet(0.0f, 0.0f, 1.0f);
    }

    forward = VNorm(forward);

    float dot = VDot(forward, dirToPlayer);
    float limit = cosf(viewHalfAngleRad_);

    // 視野角外
    if (dot < limit)
    {
        return false;
    }

    // 壁などで遮られている
    if (IsBlockedByWall(player))
    {
        return false;
    }

    return true;
}

bool EnemyBase::IsBlockedByWall(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    VECTOR start = transform_.pos;
    VECTOR end = player->GetTransform().pos;

    // 視線の高さ
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

void EnemyBase::DrawViewRange(void)
{
#ifdef _DEBUG

    VECTOR basePos = transform_.pos;

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
        cosf(leftAngle));

    VECTOR rightDir = VGet(
        sinf(rightAngle),
        0.0f,
        cosf(rightAngle));

    VECTOR leftEnd = VAdd(basePos, VScale(leftDir, viewRange_));
    VECTOR rightEnd = VAdd(basePos, VScale(rightDir, viewRange_));

    unsigned int color = isChasing_
        ? GetColor(255, 0, 0)
        : GetColor(0, 255, 0);

    DrawLine3D(basePos, leftEnd, color);
    DrawLine3D(basePos, rightEnd, color);

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
            cosf(angle));

        VECTOR p = VAdd(basePos, VScale(dir, viewRange_));

        DrawLine3D(prev, p, color);

        prev = p;
    }

#endif
}