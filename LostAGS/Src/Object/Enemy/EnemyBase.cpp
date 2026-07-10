#include <DxLib.h>
#include <math.h>
#include "../../Application.h"
#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "../Furniture/Furniture.h"
#include "../Furniture/Door.h"
#include "../Player.h"
#include "../Common/Hp/HpManager.h"
#include "../../Manager/SoundManager.h"
#include "../../Manager/SceneManager.h"
#include "EnemyBase.h"

int EnemyBase::chasingEnemyCount_ = 0;

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
    // 足音反応
    isHearingFootstep_ = false;
    hearingTimer_ = 0.0f;
    hearingDuration_ = 60.0f;
    hasPlayedDetectSE_ = false;
    wasSeeingPlayer_ = false;

    // 警戒状態
    isAlert_ = false;
    alertTimer_ = 0.0f;

 // 警戒時間
    alertDuration_ = 20.0f;

    // Enemy側の足音検知範囲
    footstepHearRangeNormal_ = 350.0f;
    footstepHearRangeAlert_ = 500.0f;

    lastHeardPos_ = VGet(0.0f, 0.0f, 0.0f);
    wasChasing_ = false;
    lastKnownPlayerPos_ = VGet(0.0f, 0.0f, 0.0f);
    isPlayerAttackHit_ = false;
    isFootstepActive_ = false;
    footstepTimer_ = 0.0f;
    footstepInterval_ = 0.45f;
    footstepRange_ = 0.0f;
    footstepSeHandle_ = -1;
    isActive_ = true;
#ifdef _DEBUG
    debugFootstepHearRange_ = footstepHearRangeNormal_;
    debugCanHearFootstep_ = false;
#endif
}

EnemyBase::~EnemyBase(void)
{

    if (isChasing_)
    {
        SetChasing(false);
    }

    ReleaseFootstep3DSound();

    HpManager::GetInstance().UnregisterHP(this);




}

void EnemyBase::Update(void)
{
    Update(nullptr);
}

void EnemyBase::Update(Player* player)
{
    if (!isActive_)
    {
            return;
    }
    // 死亡チェック
    if (isDead_)
    {
        return;
    }
    VECTOR beforeFootstepPos = transform_.pos;

#ifdef _DEBUG
    debugFootstepHearRange_ = GetCurrentFootstepHearRange();
    debugCanHearFootstep_ = false;
#endif

    float dt = SceneManager::GetInstance().GetDeltaTime();

    if (attackIntervalTimer_ > 0.0f)
    {
        attackIntervalTimer_ -= dt;

        if (attackIntervalTimer_ < 0.0f)
        {
            attackIntervalTimer_ = 0.0f;
        }
    }

    // 警戒タイマー更新
    if (isAlert_)
    {
        float dt =
            SceneManager::GetInstance().GetDeltaTime();

        alertTimer_ -= dt;

        if (alertTimer_ <= 0.0f)
        {
            isAlert_ = false;
            alertTimer_ = 0.0f;
        }
    }

    // 攻撃していないならリセット
    if (player != nullptr &&
        !player->IsAttacking())
    {
        isPlayerAttackHit_ = false;
    }

    // まだヒットしていない時だけ当たる
    if (!isPlayerAttackHit_ &&
        CheckPlayerAttack(player))
    {
        isPlayerAttackHit_ = true;

        int beforeHp = GetHP();

        Damage(10);

        if (GetHP() < beforeHp)
        {
            SoundManager::GetInstance().PlaySE(
                SoundManager::SE::HIT);
        }

        if (isDead_)
        {
            return;
        }
    }

    // プレイヤー状態取得
    bool canSeePlayer = IsPlayerInView(player);
    bool inHearRange = CanHearPlayerFootstep(player);

    // 発見SE制御
    if (canSeePlayer)
    {
        // まだ鳴らしてなくて、かつ追跡開始の瞬間
        if (!hasPlayedDetectSE_ && !isChasing_)
        {
            SoundManager::GetInstance().PlaySE(
                SoundManager::SE::DISE);

            hasPlayedDetectSE_ = true;
        }
    }

    // 足音範囲外に出たらリセット
    if (!inHearRange)
    {
        hasPlayedDetectSE_ = false;
    }

    // --------------------------------------------------
    // 追跡状態更新
    // BGMは SetChasing() の中で一括管理する
    // --------------------------------------------------
    if (canSeePlayer)
    {
        SetChasing(true);

        isHearingFootstep_ = false;
        wasChasing_ = true;

        StartAlert();

        if (player != nullptr)
        {
            lastKnownPlayerPos_ = player->GetTransform().pos;
            lastKnownPlayerPos_.y = transform_.pos.y;
        }
    }
    else
    {
        SetChasing(false);

        // 直前まで追跡していて、今見失った瞬間
        if (wasChasing_)
        {
            wasChasing_ = false;

            // 見失ったので警戒状態に入る
            StartAlert();

            // 最後に見た場所を見る
            LookAtPosition(lastKnownPlayerPos_);
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
        if (isChasing_ && IsPlayerInAttackRange(player))
        {
            // 攻撃範囲内なら近づかない
            LookAtPosition(player->GetTransform().pos);

            // 攻撃可能なら攻撃
            if (attackIntervalTimer_ <= 0.0f)
            {
                StartAttack(player);
            }
            else
            {
               
            }
        }
        else if (isChasing_)
        {
            UpdateChase(player);
        }
        else if (isHearingFootstep_)
        {
            UpdateHearFootstep(player);
        }
        else
        {
            UpdateWander(player);
        }
    }

    UpdateFootstepSound(player, beforeFootstepPos);

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
    if (!isActive_)
    {
            return;
    }

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
    (void)beforePos;

    if (player == nullptr)
    {
        return false;
    }

    const auto& furnitures = player->GetFurnitures();

    float bottomY = transform_.pos.y;
    float topY = transform_.pos.y + bodyHeight_;

    for (auto f : furnitures)
    {
        if (f == nullptr)
        {
            continue;
        }

        // -------------------------------------------------
        // 追加：扉専用処理
        // -------------------------------------------------
        Door* door = dynamic_cast<Door*>(f);

        if (door != nullptr)
        {
            const float OPEN_MARGIN = 80.0f;

            bool isNearDoor =
                door->IsEnemyNearDoor(
                    transform_.pos,
                    radius_,
                    bottomY,
                    topY,
                    OPEN_MARGIN
                );

            // 敵が扉の近くにいる時だけ開ける
            if (isNearDoor)
            {
                door->OpenByEnemy();
            }

            // 完全に開いているなら通れる
            if (door->IsPassableForEnemy())
            {
                continue;
            }

            // 閉まっている / 開き途中なら押し戻す
            if (door->ResolveEnemyCollision(
                transform_.pos,
                radius_,
                bottomY,
                topY))
            {
                return true;
            }

            continue;
        }

        // -------------------------------------------------
        // 1. Wall / Showcase など ResolveCollision を持つ家具
        // -------------------------------------------------
        VECTOR testPos = transform_.pos;

        if (f->ResolveCollision(
            testPos,
            radius_,
            bottomY,
            topY))
        {
            transform_.pos = testPos;
            return true;
        }

        // -------------------------------------------------
        // 2. Table など OBB Collider を持つ家具
        // -------------------------------------------------
        for (const auto& obb : f->GetOBBColliders())
        {
            if (obb.ResolveCollisionXZ(
                transform_.pos,
                radius_,
                bottomY,
                topY))
            {
                return true;
            }
        }

        // -------------------------------------------------
        // 3. BoxCollider 系の家具
        // -------------------------------------------------
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

    // ここで死んだ時だけ倒す
    if (HpManager::GetInstance().IsDead(this))
    {
        // 追跡中だった敵が死んだら追跡数を減らす
        if (isChasing_)
        {
            SetChasing(false);
        }

        isDead_ = true;

        isAttacking_ = false;
        isAttackHit_ = false;
        isHearingFootstep_ = false;
        isFootstepActive_ = false;

        transform_.pos.y = groundY_;

        // 死亡時だけX軸90度で寝かせる
        transform_.quaRotLocal =
            Quaternion::Euler({
                AsoUtility::Deg2RadF(90.0f),
                AsoUtility::Deg2RadF(180.0f),
                0.0f
                });

        transform_.Update();
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


    const int DIV = 64;
    const float PI = 3.1415926535f;

    for (int i = 0; i < DIV; i++)
    {
        float angle1 = 2.0f * PI * i / DIV;
        float angle2 = 2.0f * PI * (i + 1) / DIV;

        VECTOR p1 = VGet(
            center.x + sinf(angle1) * drawRange,
            center.y,
            center.z + cosf(angle1) * drawRange
        );

        VECTOR p2 = VGet(
            center.x + sinf(angle2) * drawRange,
            center.y,
            center.z + cosf(angle2) * drawRange
        );

        DrawLine3D(p1, p2, color);
    }


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

int EnemyBase::GetHP(void) const
{
    const Hp* hp =
        HpManager::GetInstance().GetHP(this);

    if (hp == nullptr)
    {
        return 0;
    }

    return hp->GetCurrent();
}

void EnemyBase::SetChasing(bool chasing)
{
    if (isChasing_ == chasing)
    {
        return;
    }

    isChasing_ = chasing;

    if (isChasing_)
    {
        chasingEnemyCount_++;

        // 追跡中の敵が1体目になった瞬間だけCHASEへ
        if (chasingEnemyCount_ == 1)
        {
            SoundManager::GetInstance().PlayBGM(
                SoundManager::BGM::CHASE);
        }
    }
    else
    {
        chasingEnemyCount_--;

        if (chasingEnemyCount_ < 0)
        {
            chasingEnemyCount_ = 0;
        }

        // 追跡中の敵が0体になったらGAMEへ戻す
        if (chasingEnemyCount_ == 0)
        {
            SoundManager::GetInstance().PlayBGM(
                SoundManager::BGM::GAME);
        }
    }
}

void EnemyBase::ResetChasingEnemyCount()
{
    chasingEnemyCount_ = 0;
}

void EnemyBase::UpdateFootstepSound(Player* player, const VECTOR& beforePos)
{
    (void)player;

    if (footstepSeHandle_ == -1)
    {
        return;
    }

    VECTOR diff =
        VSub(transform_.pos, beforePos);

    diff.y = 0.0f;

    float moveDistance =
        VSize(diff);

    // ほぼ動いていない、または攻撃中なら足音なし
    if (moveDistance < 0.5f || isAttacking_)
    {
        isFootstepActive_ = false;
        footstepTimer_ = 0.0f;
        footstepRange_ = 0.0f;
        return;
    }

    isFootstepActive_ = true;

    if (isChasing_)
    {
        footstepInterval_ = 0.28f;
        footstepRange_ = 900.0f;
    }
    else
    {
        footstepInterval_ = 0.45f;
        footstepRange_ = 500.0f;
    }

    SoundManager::GetInstance().Set3DSERadius(
        footstepSeHandle_,
        footstepRange_
    );

    footstepTimer_ +=
        SceneManager::GetInstance().GetDeltaTime();

    if (footstepTimer_ < footstepInterval_)
    {
        return;
    }

    footstepTimer_ = 0.0f;

    VECTOR soundPos = transform_.pos;
    soundPos.y += 30.0f;

    SoundManager::GetInstance().Play3DSE(
        footstepSeHandle_,
        soundPos
    );
}

void EnemyBase::InitFootstep3DSound(void)
{
    if (footstepSeHandle_ != -1)
    {
        return;
    }

    footstepSeHandle_ =
        SoundManager::GetInstance().Create3DSE(
            SoundManager::SE::E_WALK,
            900.0f
        );
}

void EnemyBase::ReleaseFootstep3DSound(void)
{
    SoundManager::GetInstance().Delete3DSE(
        footstepSeHandle_
    );
}