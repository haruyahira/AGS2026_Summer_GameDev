#include <DxLib.h>
#include <math.h>

#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "../Furniture/Furniture.h"
#include "../Player.h"
#include "../Common/Hp/HpManager.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(void) : ActorBase()
{
    speed_ = 1.0f;
    radius_ = 20.0f;
    groundY_ = -90.0f;

    isDead_ = false;
    isChasing_ = false;

    viewRange_ = 500.0f;
    viewHalfAngleRad_ = AsoUtility::Deg2RadF(45.0f);

    forwardDir_ = VGet(0.0f, 0.0f, 1.0f);

    // 攻撃用
    isAttacking_ = false;
    isAttackHit_ = false;

    attackTimer_ = 0.0f;
    attackDuration_ = 45.0f;

    attackIntervalTimer_ = 0.0f;
    attackInterval_ = 90.0f;

    attackRange_ = 90.0f;
    attackRadius_ = 35.0f;

    currentAnimType_ = -1;

    bodyHeight_ = 120.0f;
    bodyCenterOffsetY_ = 60.0f;


    maxHp_ = 1;
    attackPower_ = 1;

}

EnemyBase::~EnemyBase(void)
{
        HpManager::GetInstance().UnregisterHP(this);
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

    // 攻撃インターバル更新
    if (attackIntervalTimer_ > 0.0f)
    {
        attackIntervalTimer_ -= 1.0f;
    }

    // プレイヤーの攻撃を受けたか
    if (CheckPlayerAttack(player))
    {

        Damage(1);

        if (isDead_)
        {
            return;
        }

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

    // 攻撃中は攻撃処理を優先
    if (isAttacking_)
    {
        UpdateAttack(player);
    }
    else
    {
        // 視野内、近距離、インターバル終了なら攻撃
        if (isChasing_ &&
            IsPlayerInAttackRange(player) &&
            attackIntervalTimer_ <= 0.0f)
        {
            StartAttack(player);
        }
        else if (isChasing_)
        {
            UpdateChase(player);
        }
        else
        {
            UpdateWander(player);
        }
    }

    // アニメーション更新
    if (animationController_)
    {
        animationController_->Update();
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

    // 本体当たり判定
    DrawSphere3D(
        transform_.pos,
        radius_,
        16,
        GetColor(255, 0, 0),
        GetColor(255, 0, 0),
        FALSE);

    // 視野
    DrawViewRange();

    // 攻撃判定
    DrawAttackRange();


    // HP表示
    auto hp = HpManager::GetInstance().GetHP(this);

    if (hp != nullptr)
    {
        VECTOR hpPos = transform_.pos;
        hpPos.y += 120.0f;

        VECTOR screenPos = ConvWorldPosToScreenPos(hpPos);

        DrawFormatString(
            static_cast<int>(screenPos.x) - 45,
            static_cast<int>(screenPos.y),
            GetColor(255, 80, 80),
            "Enemy HP: %d / %d",
            hp->GetCurrent(),
            hp->GetMax()
        );

        // HPバー背景
        int barX = static_cast<int>(screenPos.x) - 40;
        int barY = static_cast<int>(screenPos.y) + 18;
        int barW = 80;
        int barH = 8;

        DrawBox(
            barX,
            barY,
            barX + barW,
            barY + barH,
            GetColor(80, 80, 80),
            TRUE);

        float hpRate = hp->GetRate();

        DrawBox(
            barX,
            barY,
            barX + static_cast<int>(barW * hpRate),
            barY + barH,
            GetColor(255, 0, 0),
            TRUE);

        // 枠
        DrawBox(
            barX,
            barY,
            barX + barW,
            barY + barH,
            GetColor(255, 255, 255),
            FALSE);
    }


#endif
}

void EnemyBase::ChangeAnimation(int animType, bool isLoop)
{
    if (animationController_ == nullptr)
    {
        return;
    }

    if (currentAnimType_ == animType)
    {
        return;
    }

    currentAnimType_ = animType;
    animationController_->Play(animType, isLoop);
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

    float attackRadius = 45.0f;

    // 敵の円柱の下端・上端
    float bottomY = transform_.pos.y;
    float topY = transform_.pos.y + bodyHeight_;

    // 攻撃位置のYを円柱の高さ範囲に丸める
    float closestY = attackPos.y;

    if (closestY < bottomY)
    {
        closestY = bottomY;
    }
    else if (closestY > topY)
    {
        closestY = topY;
    }

    // XZ平面で敵中心から攻撃位置への差
    float diffX = attackPos.x - transform_.pos.x;
    float diffZ = attackPos.z - transform_.pos.z;

    float distXZ = sqrtf(diffX * diffX + diffZ * diffZ);

    // 円柱側の最近点を求める
    VECTOR closestPoint = transform_.pos;
    closestPoint.y = closestY;

    if (distXZ > 0.001f)
    {
        float nx = diffX / distXZ;
        float nz = diffZ / distXZ;

        closestPoint.x += nx * radius_;
        closestPoint.z += nz * radius_;
    }

    // 攻撃球中心と敵円柱表面の最近点の距離
    VECTOR diff = VSub(attackPos, closestPoint);
    float distSq = VDot(diff, diff);

    return distSq <= attackRadius * attackRadius;
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
    // beforePosは呼び出し側で戻すために使う
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

    if (dot < limit)
    {
        return false;
    }

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

    start.y += 60.0f;
    end.y += 60.0f;

    const auto& furnitures = player->GetFurnitures();

    for (auto f : furnitures)
    {
        if (f == nullptr)
        {
            continue;
        }

        if (f->IsBlockingSight(start, end))
        {
            return true;
        }
    }

    return false;
}

bool EnemyBase::IsPlayerInAttackRange(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    VECTOR playerPos = player->GetTransform().pos;

    VECTOR diff = VSub(playerPos, transform_.pos);
    diff.y = 0.0f;

    float distSq = VDot(diff, diff);

    return distSq <= attackRange_ * attackRange_;
}

bool EnemyBase::IsEnemyAttackHitPlayer(Player* player)
{
    if (player == nullptr)
    {
        return false;
    }

    VECTOR attackPos = GetAttackPos();
    VECTOR playerPos = player->GetTransform().pos;

    VECTOR diff = VSub(playerPos, attackPos);
    diff.y = 0.0f;

    float distSq = VDot(diff, diff);

    float playerRadius = 15.0f;
    float hitRange = attackRadius_ + playerRadius;

    return distSq <= hitRange * hitRange;
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

void EnemyBase::DrawAttackRange(void)
{
#ifdef _DEBUG

    if (isAttacking_)
    {
        DrawSphere3D(
            GetAttackPos(),
            attackRadius_,
            16,
            GetColor(255, 150, 0),
            GetColor(255, 150, 0),
            FALSE);
    }

#endif
}

void EnemyBase::InitHP(int maxHp, float invincibleFrame)
{
    maxHp_ = maxHp;

    HpManager::GetInstance().RegisterHP(
        this,
        maxHp_,
        invincibleFrame);
}

void EnemyBase::Damage(int damage)
{
    bool isDamaged =
        HpManager::GetInstance().Damage(this, damage);

    if (!isDamaged)
    {
        return;
    }

    printfDx("Enemy Damage : %d\n", damage);

    if (HpManager::GetInstance().IsDead(this))
    {
        isDead_ = true;

        printfDx("Enemy Dead\n");
    }
}