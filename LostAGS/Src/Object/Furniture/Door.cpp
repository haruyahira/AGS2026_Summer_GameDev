#include <cmath>
#include "Door.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SoundManager.h"
#include "../Player.h"
#include "../../Application.h"

// =========================
// 補助関数
// =========================

static VECTOR RotateY(const VECTOR& v, float rad)
{
    float c = cosf(rad);
    float s = sinf(rad);

    return VGet(
        v.x * c - v.z * s,
        v.y,
        v.x * s + v.z * c
    );
}

static float DotXZ(const VECTOR& a, const VECTOR& b)
{
    return a.x * b.x + a.z * b.z;
}

static VECTOR ClosestPointOnDoorXZ(
    const VECTOR& point,
    const DoorCollision& col
)
{
    VECTOR d = VSub(point, col.center);

    float x = DotXZ(d, col.axisX);
    float z = DotXZ(d, col.axisZ);

    if (x < -col.half.x) x = -col.half.x;
    if (x > col.half.x) x = col.half.x;

    if (z < -col.half.z) z = -col.half.z;
    if (z > col.half.z) z = col.half.z;

    VECTOR result = col.center;

    result = VAdd(
        result,
        VScale(col.axisX, x)
    );

    result = VAdd(
        result,
        VScale(col.axisZ, z)
    );

    return result;
}

// =========================
// コンストラクタ
// =========================

Door::Door(
    const Transform* trans,
    float rotY,
    float hingeSide,
    float openSign
)
    : Furniture(NAME::DOOR, trans)
{
    baseRotY_ = rotY;
    hingeSide_ = hingeSide;
    openSign_ = openSign;
}

// =========================
// 初期化
// =========================

void Door::Init()
{
    colliders_.clear();

    animAttachIndex_ =
        MV1AttachAnim(trans_.modelId, 0);

    if (animAttachIndex_ != -1)
    {
        animTotalTime_ =
            MV1GetAttachAnimTotalTime(
                trans_.modelId,
                animAttachIndex_
            );

        animTime_ = 0.0f;

        MV1SetAttachAnimTime(
            trans_.modelId,
            animAttachIndex_,
            animTime_
        );
    }

    state_ = DoorState::Closed;

    // =========================
    // 扉当たり判定の初期設定
    // =========================
 // =========================
// 扉当たり判定の初期設定
// =========================

// 先にサイズを設定する
    doorWidth_ = 120.0f;
    doorHeight_ = 260.0f;
    doorThickness_ = 30.0f;

    playerRadius_ = 35.0f;

    closedRotY_ = baseRotY_;
    openAngle_ = 0.0f;


    // ドア中心のローカルオフセット
    // 今のコードでは 0 にしているので、全部 0 に統一
    VECTOR localDoorCenterOffset =
        VGet(0.0f, 0.0f, 0.0f);

    VECTOR worldDoorCenterOffset =
        RotateY(localDoorCenterOffset, closedRotY_);

    VECTOR closedDoorCenter =
        VAdd(trans_.pos, worldDoorCenterOffset);

    VECTOR closedAxisX =
        RotateY(VGet(1.0f, 0.0f, 0.0f), closedRotY_);

    closedAxisX = VNorm(closedAxisX);

    // 蝶番位置
    hingePos_ =
        VAdd(
            closedDoorCenter,
            VScale(closedAxisX, hingeSide_ * doorWidth_ * 0.5f)
        );

    isCollisionActive_ = true;

    UpdateCollision();
}

// =========================
// プレイヤーなしUpdate
// =========================

void Door::Update(void)
{
    if (animAttachIndex_ == -1)
    {
        return;
    }

    UpdateAnimation();
    UpdateCollision();
}

// =========================
// プレイヤーありUpdate
// =========================

void Door::Update(Player& player)
{
    bool canInteract = CanInteract(player);

    auto& ins = InputManager::GetInstance();

    bool isOpenTrigger =
        ins.IsTrgDown(KEY_INPUT_F) ||
        ins.IsPadBtnTrgDown(
            InputManager::JOYPAD_NO::PAD1,
            InputManager::JOYPAD_BTN::LEFT); 

    if (canInteract && isOpenTrigger)
    {
        auto& snd = SoundManager::GetInstance();

        if (state_ == DoorState::Closed ||
            state_ == DoorState::Closing)
        {
            state_ = DoorState::Opening;

            snd.SetSEVolume(255);
            snd.PlaySE(SoundManager::SE::DOOROP);
        }
        else if (state_ == DoorState::Open ||
            state_ == DoorState::Opening)
        {
            state_ = DoorState::Closing;

            snd.SetSEVolume(255);
            snd.PlaySE(SoundManager::SE::DOOROP);
        }
    }

    if (animAttachIndex_ != -1)
    {
        UpdateAnimation();
    }

    UpdateCollision();

    CheckPlayerCollision(player);
}

// =========================
// アニメーション更新
// =========================

void Door::UpdateAnimation()
{
    switch (state_)
    {
    case DoorState::Opening:
        animTime_ += animSpeed_;

        if (animTime_ >= animTotalTime_)
        {
            animTime_ = animTotalTime_;
            state_ = DoorState::Open;
        }
        break;

    case DoorState::Closing:
        animTime_ -= animSpeed_;

        if (animTime_ <= 0.0f)
        {
            animTime_ = 0.0f;
            state_ = DoorState::Closed;
        }
        break;

    case DoorState::Closed:
    case DoorState::Open:
        break;
    }

    MV1SetAttachAnimTime(
        trans_.modelId,
        animAttachIndex_,
        animTime_
    );

    // =========================
    // アニメーション時間から当たり判定の角度を作る
    // =========================

    float rate = 0.0f;

    if (animTotalTime_ > 0.001f)
    {
        rate = animTime_ / animTotalTime_;
    }

    // 90度開く想定
    // 逆方向に開くなら -DX_PI_F / 2.0f にしてください。
    openAngle_ = DX_PI_F / 2.0f * rate;

    // 開ききったら判定を消したい場合はこちらを使う
    // 今回は開いた扉にも当たり判定を残す想定
    isCollisionActive_ = true;

    /*
    if (rate >= 0.95f)
    {
        isCollisionActive_ = false;
    }
    else
    {
        isCollisionActive_ = true;
    }
    */
}

// =========================
// 描画
// =========================

void Door::Draw(void)
{
    trans_.Update();

    MV1DrawModel(trans_.modelId);
}

// =========================
// インタラクト判定
// =========================

bool Door::CanInteract(const Player& player) const
{
    VECTOR playerPos = player.GetPos();

    playerPos.y = collision_.center.y;

    const float interactDistance = 150.0f;

    VECTOR doorCenter = collision_.center;
    doorCenter.y = playerPos.y;

    VECTOR doorAxis = collision_.axisX;
    doorAxis.y = 0.0f;

    if (VSize(doorAxis) < 0.001f)
    {
        doorAxis = VGet(1.0f, 0.0f, 0.0f);
    }

    doorAxis = VNorm(doorAxis);

    const float doorHalfWidth = collision_.half.x;

    VECTOR doorA = VAdd(
        doorCenter,
        VScale(doorAxis, -doorHalfWidth)
    );

    VECTOR doorB = VAdd(
        doorCenter,
        VScale(doorAxis, doorHalfWidth)
    );

    VECTOR ab = VSub(doorB, doorA);
    VECTOR ap = VSub(playerPos, doorA);

    float abLenSq = VDot(ab, ab);

    if (abLenSq < 0.001f)
    {
        return false;
    }

    float t = VDot(ap, ab) / abLenSq;

    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }

    VECTOR closestPoint = VAdd(
        doorA,
        VScale(ab, t)
    );

    VECTOR toDoor = VSub(closestPoint, playerPos);
    toDoor.y = 0.0f;

    float distance = VSize(toDoor);

    if (distance > interactDistance)
    {
        return false;
    }

    // 向きは見ない
    // 近ければ前側・後ろ側どちらからでも開けられる
    return true;
}
// =========================
// UI描画
// =========================

bool Door::DrawInteractUI(const Player& player) const
{
    if (!CanInteract(player))
    {
        return false;
    }

    int screenW, screenH;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    const float boxWRate = 0.22f;
    const float boxHRate = 0.07f;

    int boxW =
        static_cast<int>(screenW * boxWRate);

    int boxH =
        static_cast<int>(screenH * boxHRate);

    if (boxW < 220)
    {
        boxW = 220;
    }

    if (boxH < 50)
    {
        boxH = 50;
    }

    const float posXRate = 0.5f;
    const float posYRate = 0.5f;

    int centerX =
        static_cast<int>(screenW * posXRate);

    int centerY =
        static_cast<int>(screenH * posYRate);

    int x = centerX - boxW / 2;
    int y = centerY - boxH / 2;

    const char* text = "";

    switch (state_)
    {
    case DoorState::Closed:
    case DoorState::Closing:
        text = "F 開ける";
        break;

    case DoorState::Open:
    case DoorState::Opening:
        text = "F 閉める";
        break;
    }

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        170
    );

    DrawBox(
        x,
        y,
        x + boxW,
        y + boxH,
        GetColor(0, 0, 0),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    DrawBox(
        x,
        y,
        x + boxW,
        y + boxH,
        GetColor(255, 255, 255),
        FALSE
    );

    int textX =
        x + static_cast<int>(boxW * 0.34f);

    int textY =
        y + static_cast<int>(boxH * 0.32f);

    DrawString(
        textX,
        textY,
        text,
        GetColor(255, 255, 255)
    );

    return true;
}

// =========================
// インタラクト範囲デバッグ
// =========================

void Door::DebugDrawInteractRange(
    const Player& player
) const
{
    VECTOR playerPos = player.GetPos();

    playerPos.y = collision_.center.y;

    const float interactDistance = 220.0f;
    const float doorHalfWidth = collision_.half.x;

    VECTOR doorCenter = collision_.center;
    doorCenter.y = playerPos.y;

    VECTOR doorAxis = collision_.axisX;
    doorAxis.y = 0.0f;

    if (VSize(doorAxis) < 0.001f)
    {
        doorAxis = VGet(1.0f, 0.0f, 0.0f);
    }

    doorAxis = VNorm(doorAxis);

    VECTOR doorA = VAdd(
        doorCenter,
        VScale(doorAxis, -doorHalfWidth)
    );

    VECTOR doorB = VAdd(
        doorCenter,
        VScale(doorAxis, doorHalfWidth)
    );

    VECTOR ab = VSub(doorB, doorA);
    VECTOR ap = VSub(playerPos, doorA);

    float abLenSq = VDot(ab, ab);

    if (abLenSq < 0.001f)
    {
        return;
    }

    float t = VDot(ap, ab) / abLenSq;

    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }

    VECTOR closestPoint = VAdd(
        doorA,
        VScale(ab, t)
    );

    VECTOR toDoor = VSub(
        closestPoint,
        playerPos
    );

    toDoor.y = 0.0f;

    float distance = VSize(toDoor);

    bool distanceOK =
        distance <= interactDistance;

    bool canInteract =
        distanceOK;

    int okColor = GetColor(0, 255, 0);
    int ngColor = GetColor(255, 0, 0);

    int lineColor =
        canInteract ? okColor : ngColor;

    SetUseLighting(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetUseZBuffer3D(FALSE);

    // ドアのインタラクト判定ライン
    DrawLine3D(
        doorA,
        doorB,
        GetColor(0, 255, 255)
    );

    // ライン端
    DrawSphere3D(
        doorA,
        20.0f,
        12,
        GetColor(255, 255, 0),
        GetColor(255, 255, 0),
        TRUE
    );

    DrawSphere3D(
        doorB,
        20.0f,
        12,
        GetColor(0, 128, 255),
        GetColor(0, 128, 255),
        TRUE
    );

    // プレイヤーに一番近い点
    DrawSphere3D(
        closestPoint,
        30.0f,
        12,
        GetColor(255, 0, 255),
        GetColor(255, 0, 255),
        TRUE
    );

    // プレイヤーから最近点への線
    DrawLine3D(
        playerPos,
        closestPoint,
        lineColor
    );

    DrawFormatString(
        20,
        520,
        GetColor(0, 255, 255),
        "Cyan Line = Door Interact Line"
    );

    DrawFormatString(
        20,
        545,
        GetColor(255, 0, 255),
        "Magenta Sphere = Closest Check Point"
    );

    DrawFormatString(
        20,
        570,
        lineColor,
        "Distance: %.1f / %.1f  %s",
        distance,
        interactDistance,
        distanceOK ? "OK" : "NG"
    );

    DrawFormatString(
        20,
        595,
        lineColor,
        canInteract ? "Door Interact: OK" : "Door Interact: NG"
    );

    bool actualCanInteract =
        CanInteract(player);

    DrawFormatString(
        20,
        620,
        actualCanInteract ? GetColor(0, 255, 0) : GetColor(255, 0, 0),
        actualCanInteract ? "CanInteract(): TRUE" : "CanInteract(): FALSE"
    );

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}
// =========================
// 扉当たり判定更新
// =========================

void Door::UpdateCollision()
{
    float rotY =
        closedRotY_ + openSign_ * openAngle_;

    VECTOR axisX =
        RotateY(
            VGet(1.0f, 0.0f, 0.0f),
            rotY
        );

    VECTOR axisZ =
        RotateY(
            VGet(0.0f, 0.0f, 1.0f),
            rotY
        );

    axisX = VNorm(axisX);
    axisZ = VNorm(axisZ);

    // hingeSide_ に対応した中心位置
    VECTOR centerOffset =
        VScale(
            axisX,
            -hingeSide_ * doorWidth_ * 0.5f
        );

    collision_.center =
        VAdd(
            hingePos_,
            centerOffset
        );

    collision_.axisX = axisX;
    collision_.axisZ = axisZ;

    collision_.half =
        VGet(
            doorWidth_ * 0.5f,
            doorHeight_ * 0.5f,
            doorThickness_ * 0.5f
        );
}

// =========================
// プレイヤーと扉のヒット判定
// =========================

bool Door::IsHitPlayer(
    const Player& player
) const
{
    VECTOR playerPos =
        player.GetPos();

    VECTOR closest =
        ClosestPointOnDoorXZ(
            playerPos,
            collision_
        );

    float dx =
        playerPos.x - closest.x;

    float dz =
        playerPos.z - closest.z;

    float distSq =
        dx * dx + dz * dz;

    return distSq <=
        playerRadius_ * playerRadius_;
}

// =========================
// プレイヤー押し戻し
// =========================

void Door::CheckPlayerCollision(
    Player& player
)
{
    if (!isCollisionActive_)
    {
        return;
    }

    VECTOR playerPos =
        player.GetPos();

    VECTOR closest =
        ClosestPointOnDoorXZ(
            playerPos,
            collision_
        );

    float dx =
        playerPos.x - closest.x;

    float dz =
        playerPos.z - closest.z;

    float distSq =
        dx * dx + dz * dz;

    float radiusSq =
        playerRadius_ * playerRadius_;

    if (distSq > radiusSq)
    {
        return;
    }

    float dist =
        sqrtf(distSq);

    // 完全に中心が重なった時の保険
    if (dist < 0.001f)
    {
        VECTOR pushDir =
            collision_.axisZ;

        VECTOR newPos =
            VAdd(
                playerPos,
                VScale(pushDir, playerRadius_)
            );

        player.SetPos(newPos);
        return;
    }

    float push =
        playerRadius_ - dist;

    VECTOR pushDir =
        VGet(
            dx / dist,
            0.0f,
            dz / dist
        );

    VECTOR newPos =
        VAdd(
            playerPos,
            VScale(pushDir, push)
        );

    player.SetPos(newPos);
}

// =========================
// 当たり判定デバッグ描画
// =========================

void Door::DebugDrawCollision() const
{
    SetUseLighting(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetUseZBuffer3D(FALSE);

    int colorBox = GetColor(0, 255, 0);
    int colorX = GetColor(255, 0, 0);
    int colorZ = GetColor(0, 255, 255);
    int colorCenter = GetColor(255, 255, 0);

    VECTOR center = collision_.center;

    VECTOR axisX = collision_.axisX;
    VECTOR axisZ = collision_.axisZ;

    float hx = collision_.half.x;
    float hz = collision_.half.z;

    // =========================
    // XZ平面の4隅
    // =========================

    VECTOR p0 = VAdd(
        VAdd(center, VScale(axisX, -hx)),
        VScale(axisZ, -hz)
    );

    VECTOR p1 = VAdd(
        VAdd(center, VScale(axisX, hx)),
        VScale(axisZ, -hz)
    );

    VECTOR p2 = VAdd(
        VAdd(center, VScale(axisX, hx)),
        VScale(axisZ, hz)
    );

    VECTOR p3 = VAdd(
        VAdd(center, VScale(axisX, -hx)),
        VScale(axisZ, hz)
    );

    // 少し上に浮かせると床と重なって見えやすい
    p0.y += 10.0f;
    p1.y += 10.0f;
    p2.y += 10.0f;
    p3.y += 10.0f;

    VECTOR c = center;
    c.y += 10.0f;

    // =========================
    // 当たり判定の外枠
    // =========================

    DrawLine3D(p0, p1, colorBox);
    DrawLine3D(p1, p2, colorBox);
    DrawLine3D(p2, p3, colorBox);
    DrawLine3D(p3, p0, colorBox);

    // =========================
    // 中心線
    // =========================

    VECTOR x1 = VAdd(c, VScale(axisX, hx));
    VECTOR x2 = VAdd(c, VScale(axisX, -hx));

    VECTOR z1 = VAdd(c, VScale(axisZ, hz));
    VECTOR z2 = VAdd(c, VScale(axisZ, -hz));

    DrawLine3D(x1, x2, colorX);
    DrawLine3D(z1, z2, colorZ);

    // =========================
    // 中心点
    // =========================

    DrawSphere3D(
        c,
        18.0f,
        12,
        colorCenter,
        colorCenter,
        TRUE
    );

    // =========================
    // 4隅にも球を出す
    // =========================

    DrawSphere3D(p0, 10.0f, 8, colorBox, colorBox, TRUE);
    DrawSphere3D(p1, 10.0f, 8, colorBox, colorBox, TRUE);
    DrawSphere3D(p2, 10.0f, 8, colorBox, colorBox, TRUE);
    DrawSphere3D(p3, 10.0f, 8, colorBox, colorBox, TRUE);

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}

void Door::OpenByEnemy(void)
{
    // 閉まっている、または閉まり中なら開ける
    if (state_ == DoorState::Closed ||
        state_ == DoorState::Closing)
    {
        state_ = DoorState::Opening;

        auto& snd = SoundManager::GetInstance();
        snd.SetSEVolume(255);
        snd.PlaySE(SoundManager::SE::DOOROP);
    }
}

bool Door::IsPassableForEnemy(void) const
{
    // 完全に開いたら敵は通れる
    return state_ == DoorState::Open;
}

bool Door::ResolveEnemyCollision(
    VECTOR& enemyPos,
    float enemyRadius,
    float bottomY,
    float topY
) const
{
    if (!isCollisionActive_)
    {
        return false;
    }

    // 完全に開いているなら当たり判定なし
    if (IsPassableForEnemy())
    {
        return false;
    }

    // 高さ判定
    float doorBottom = collision_.center.y - collision_.half.y;
    float doorTop = collision_.center.y + collision_.half.y;

    if (topY < doorBottom || bottomY > doorTop)
    {
        return false;
    }

    VECTOR closest =
        ClosestPointOnDoorXZ(
            enemyPos,
            collision_
        );

    float dx = enemyPos.x - closest.x;
    float dz = enemyPos.z - closest.z;

    float distSq = dx * dx + dz * dz;
    float radiusSq = enemyRadius * enemyRadius;

    if (distSq > radiusSq)
    {
        return false;
    }

    float dist = sqrtf(distSq);

    // 完全に重なった時の保険
    if (dist < 0.001f)
    {
        VECTOR pushDir = collision_.axisZ;

        enemyPos = VAdd(
            enemyPos,
            VScale(pushDir, enemyRadius)
        );

        return true;
    }

    float push = enemyRadius - dist;

    VECTOR pushDir =
        VGet(
            dx / dist,
            0.0f,
            dz / dist
        );

    enemyPos =
        VAdd(
            enemyPos,
            VScale(pushDir, push)
        );

    return true;
}

bool Door::IsEnemyNearDoor(
    const VECTOR& enemyPos,
    float enemyRadius,
    float bottomY,
    float topY,
    float margin
) const
{
    // 高さ判定
    float doorBottom = collision_.center.y - collision_.half.y;
    float doorTop = collision_.center.y + collision_.half.y;

    if (topY < doorBottom || bottomY > doorTop)
    {
        return false;
    }

    VECTOR closest =
        ClosestPointOnDoorXZ(
            enemyPos,
            collision_
        );

    float dx = enemyPos.x - closest.x;
    float dz = enemyPos.z - closest.z;

    float distSq = dx * dx + dz * dz;

    float checkRadius =
        enemyRadius + margin;

    return distSq <= checkRadius * checkRadius;
}