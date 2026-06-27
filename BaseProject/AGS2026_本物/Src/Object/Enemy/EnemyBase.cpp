#include <DxLib.h>
#include <math.h>

#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "../Furniture/Furniture.h"
#include "../Player.h"
#include "../Common/Hp/HpManager.h"
#include "../../Manager/SoundManager.h"
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
    // プレイヤーの攻撃を受ける判定用
    playerAttackHitRadius_ = 50.0f;


    attackIntervalTimer_ = 0.0f;
    attackInterval_ = 90.0f;

    attackRange_ = 90.0f;
    attackRadius_ = 35.0f;

    currentAnimType_ = -1;

    bodyHeight_ = 120.0f;
    bodyCenterOffsetY_ = 60.0f;


    maxHp_ = 1;
    attackPower_ = 1;
    // 足音反応
    isHearingFootstep_ = false;
    hearingTimer_ = 0.0f;
    hearingDuration_ = 60.0f;

    // 警戒状態
    isAlert_ = false;
    alertTimer_ = 0.0f;

    // 警戒時間
    alertDuration_ = 600.0f;

    // Enemy側の足音検知範囲
    footstepHearRangeNormal_ = 450.0f;
    footstepHearRangeAlert_ = 800.0f;

    lastHeardPos_ = VGet(0.0f, 0.0f, 0.0f);
    wasChasing_ = false;
    lastKnownPlayerPos_ = VGet(0.0f, 0.0f, 0.0f);
#ifdef _DEBUG
    debugFootstepHearRange_ = footstepHearRangeNormal_;
    debugCanHearFootstep_ = false;
#endif
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

#ifdef _DEBUG

    debugFootstepHearRange_ = GetCurrentFootstepHearRange();
    debugCanHearFootstep_ = false;

#endif


    // 攻撃インターバル更新
    if (attackIntervalTimer_ > 0.0f)
    {
        attackIntervalTimer_ -= 1.0f;
    }

    // 警戒タイマー更新
    if (isAlert_)
    {
        alertTimer_ -= 1.0f;

        if (alertTimer_ <= 0.0f)
        {
            isAlert_ = false;
            alertTimer_ = 0.0f;
        }
    }

    // プレイヤーの攻撃を受けたか
    if (CheckPlayerAttack(player))
    {
        Damage(16);

        // ヒットSEを鳴らす
        player->OnAttackHit();

        if (isDead_)
        {
            return;
        }
    }

    // 視野判定
    bool canSeePlayer = IsPlayerInView(player);

    if (canSeePlayer)
    {

        // 今見つけた瞬間だけ
        if (!isChasing_)
        {
			// 見つけたSE再生
            SoundManager::GetInstance().PlaySE("Disc");

        }

        // 追跡中も警戒状態維持
        StartAlert();

        isChasing_ = true;
        isHearingFootstep_ = false;

        wasChasing_ = true;

        if (player != nullptr)
        {
            lastKnownPlayerPos_ = player->GetTransform().pos;
            lastKnownPlayerPos_.y = transform_.pos.y;
        }

        // 見えている間は警戒を消さない
        // 追跡中も、警戒タイマーは別で進む
    }
    else
    {
        isChasing_ = false;

        // 直前まで追跡していて、今見失った瞬間
        if (wasChasing_)
        {
            wasChasing_ = false;

            // 見失ったので警戒状態に入る
            StartAlert();

            // 最後に見た場所を見る
            LookAtPosition(lastKnownPlayerPos_);

#ifdef _DEBUG
            printfDx("Enemy Lost Player -> Alert Start\n");
#endif
        }

        // 視界に入っていない時だけ足音を聞く
        if (CanHearPlayerFootstep(player))
        {
            StartHearFootstep(player);
        }
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
        else if (isHearingFootstep_)
        {
            // 足音を聞いた直後だけ、その方向を見る
            UpdateHearFootstep(player);
        }
        else
        {
            // 警戒中でも巡回する
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

#ifdef _DEBUG
        //printfDx("Enemy Hit Player\n");
#endif
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


    // 足音検知範囲
    DrawFootstepHearRangeDebug();


    // 足音を聞いている時のデバッグ表示
    if (isHearingFootstep_)
    {
        VECTOR enemyPos = transform_.pos;
        enemyPos.y += 40.0f;

        VECTOR heardPos = lastHeardPos_;
        heardPos.y += 40.0f;

        DrawLine3D(
            enemyPos,
            heardPos,
            GetColor(255, 255, 0)
        );

        VECTOR screenPos = ConvWorldPosToScreenPos(enemyPos);

        DrawFormatString(
            static_cast<int>(screenPos.x) - 50,
            static_cast<int>(screenPos.y) - 20,
            GetColor(255, 255, 0),
            "HEARD FOOTSTEP"
        );
    }
    


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

    if (!player->IsAttackHitTiming())
    {
        return false;
    }

    if (player->IsAttackHit())
    {
        return false;
    }

    VECTOR attackPos = player->GetAttackPos();

    float attackRadius = 45.0f;

    float bottomY = transform_.pos.y;
    float topY = transform_.pos.y + bodyHeight_;

    float closestY = attackPos.y;

    if (closestY < bottomY)
    {
        closestY = bottomY;
    }
    else if (closestY > topY)
    {
        closestY = topY;
    }

    float diffX = attackPos.x - transform_.pos.x;
    float diffZ = attackPos.z - transform_.pos.z;

    float distXZ = sqrtf(diffX * diffX + diffZ * diffZ);

    VECTOR closestPoint = transform_.pos;
    closestPoint.y = closestY;

    if (distXZ > 0.001f)
    {
        float nx = diffX / distXZ;
        float nz = diffZ / distXZ;

        closestPoint.x += nx * playerAttackHitRadius_;
        closestPoint.z += nz * playerAttackHitRadius_;
    }

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

#ifdef _DEBUG
    //printfDx("Enemy Damage : %d\n", damage);
#endif

    if (HpManager::GetInstance().IsDead(this))
    {
        isDead_ = true;

        isChasing_ = false;
        wasChasing_ = false;
        isAlert_ = false;
        isHearingFootstep_ = false;


#ifdef _DEBUG
       // printfDx("Enemy Dead\n");
#endif
    }
}


VECTOR EnemyBase::GetPos(void) const
{
    return transform_.pos;
}

bool EnemyBase::CanHearPlayerFootstep(Player* player)
{
#ifdef _DEBUG
    debugFootstepHearRange_ = GetCurrentFootstepHearRange();
    debugCanHearFootstep_ = false;
#endif

    if (player == nullptr)
    {
        return false;
    }

    // プレイヤーが足音を出していないなら聞こえない
    if (!player->IsFootstepActive())
    {
        return false;
    }

    VECTOR soundPos = player->GetFootstepPos();

    VECTOR diff = VSub(soundPos, transform_.pos);
    diff.y = 0.0f;

    float distSq = VDot(diff, diff);

    // Enemy側の固定検知範囲を使う
    float hearRange = GetCurrentFootstepHearRange();

    bool canHear = distSq <= hearRange * hearRange;

#ifdef _DEBUG
    debugFootstepHearRange_ = hearRange;
    debugCanHearFootstep_ = canHear;
#endif

    return canHear;
}void EnemyBase::StartHearFootstep(Player* player)
{
    if (player == nullptr)
    {
        return;
    }

    isHearingFootstep_ = true;
    hearingTimer_ = hearingDuration_;

    lastHeardPos_ = player->GetFootstepPos();
    lastHeardPos_.y = transform_.pos.y;

    // 足音を聞いたので警戒状態に入る
    StartAlert();

    // 足音がした方向を向く
    LookAtPosition(lastHeardPos_);
}

void EnemyBase::UpdateHearFootstep(Player* player)
{
    // IDLEにする
    // EnemyNormal の ANIM_TYPE::IDLE が 0 なので 0 を指定
    ChangeAnimation(0, true);

    // 足音がまだ聞こえているなら、最新の足音位置へ向き直す
    if (CanHearPlayerFootstep(player))
    {
        lastHeardPos_ = player->GetFootstepPos();
        lastHeardPos_.y = transform_.pos.y;

        // 聞こえ続けている間は聞き耳時間を更新
        hearingTimer_ = hearingDuration_;

        // 警戒時間も延長
        StartAlert();
    }

    LookAtPosition(lastHeardPos_);

    hearingTimer_ -= 1.0f;

    if (hearingTimer_ <= 0.0f)
    {
        isHearingFootstep_ = false;
    }
}

void EnemyBase::StartAlert(void)
{
    isAlert_ = true;
    alertTimer_ = alertDuration_;
}

void EnemyBase::LookAtPosition(const VECTOR& targetPos)
{
    VECTOR toTarget = VSub(targetPos, transform_.pos);
    toTarget.y = 0.0f;

    if (VSize(toTarget) < 0.001f)
    {
        return;
    }

    VECTOR dir = VNorm(toTarget);

    forwardDir_ = dir;

    float angleY = atan2f(dir.x, dir.z);

    transform_.quaRot =
        Quaternion::AngleAxis(angleY, AsoUtility::AXIS_Y);
}

#ifdef _DEBUG
void EnemyBase::DrawFootstepHearRangeDebug(void)
{
    float drawRange = GetCurrentFootstepHearRange();

    if (drawRange <= 0.0f)
    {
        return;
    }

    unsigned int color = GetColor(0, 180, 255);

    // 通常時：水色
    if (!isAlert_)
    {
        color = GetColor(0, 180, 255);
    }

    // 警戒中：オレンジ
    if (isAlert_)
    {
        color = GetColor(255, 160, 0);
    }

    // 実際に足音を聞けている時：黄色

    if (debugCanHearFootstep_)
    {
        color = GetColor(255, 255, 0);
    }

    VECTOR center = transform_.pos;

    // 地面と被ると見えにくいので少し上げる
    center.y += 15.0f;

    VECTOR pos1 = center;
    VECTOR pos2 = center;

    // 薄い円柱にして、円として見せる
    pos2.y += 3.0f;

    // デバッグ描画用
    SetUseLighting(FALSE);
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);

    DrawCylinder3D(
        pos1,
        pos2,
        drawRange,
        64,
        color,
        color,
        FALSE
    );

    // 描画設定を戻す
    SetWriteZBuffer3D(TRUE);
    SetUseZBuffer3D(TRUE);
    SetUseLighting(TRUE);

    VECTOR textPos = transform_.pos;
    textPos.y += 150.0f;

    VECTOR screenPos = ConvWorldPosToScreenPos(textPos);

    DrawFormatString(
        (int)screenPos.x - 90,
        (int)screenPos.y,
        color,
        "Hear Range: %.1f",
        drawRange
    );

    if (isAlert_)
    {
        DrawFormatString(
            (int)screenPos.x - 90,
            (int)screenPos.y + 18,
            GetColor(255, 160, 0),
            "ALERT %.0f",
            alertTimer_
        );
    }
}
#endif

float EnemyBase::GetCurrentFootstepHearRange(void) const
{
    if (isAlert_)
    {
        return footstepHearRangeAlert_;
    }

    return footstepHearRangeNormal_;
}

bool EnemyBase::IsDead(void) const
{
    return isDead_;
}