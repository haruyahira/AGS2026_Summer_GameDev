#include "Trashcan.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SoundManager.h"
#include "../Player.h"

Trashcan::Trashcan(const Transform* trans)
    : Furniture(NAME::TRANSCAN, trans)
{
}

void Trashcan::Init()
{
    colliders_.clear();



    animAttachIndex_ = MV1AttachAnim(trans_.modelId, 0);

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

    state_ = TrashcanState::Closed;

    // =========================
    // ゴミ箱の当たり判定
    // =========================
   // =========================
// ゴミ箱の外側
// =========================
    outerHalf_ = VGet(
        220.0f,   // 横幅
        120.0f,   // 高さ
        110.0f    // 奥行き
    );

    outerOffset_ = VGet(
        0.0f,
        80.0f,
        0.0f
    );

    // =========================
    // ゴミ箱の中に入れる空洞
    // この範囲にいる時は押し戻さない
    // =========================
    innerHalf_ = VGet(
        155.0f,   // 内側の横幅
        100.0f,   // 内側の高さ
        70.0f     // 内側の奥行き
    );

    innerOffset_ = VGet(
        0.0f,
        85.0f,
        0.0f
    );
}

void Trashcan::Update(void)
{
    UpdateAnimation();
}

void Trashcan::Update(Player& player)
{
    bool canInteract = CanInteract(player);

    auto& ins = InputManager::GetInstance();

    bool isInteractTrigger =
        ins.IsTrgDown(KEY_INPUT_F) ||
        ins.IsPadBtnTrgDown(
            InputManager::JOYPAD_NO::PAD1,
            InputManager::JOYPAD_BTN::LEFT);

    if (canInteract && isInteractTrigger)
    {
        auto& snd = SoundManager::GetInstance();

        if (state_ == TrashcanState::Closed ||
            state_ == TrashcanState::Closing)
        {
            state_ = TrashcanState::Opening;

            snd.SetSEVolume(255);
            snd.PlaySE(SoundManager::SE::DOOROP);
        }
        else if (state_ == TrashcanState::Open ||
            state_ == TrashcanState::Opening)
        {
            state_ = TrashcanState::Closing;

            snd.SetSEVolume(255);
            snd.PlaySE(SoundManager::SE::DOOROP);
        }
    }

    UpdateAnimation();

    // 先に中にいるかを保存
    bool isInsideBeforeCollision =
        IsPlayerInsideInner(player);

    // ゴミ箱本体の押し戻し
    CheckPlayerCollision(player);

    // 中にいて、閉じているなら隠れる
    if ((isInsideBeforeCollision || IsPlayerInsideInner(player)) &&
        IsClosedState())
    {
        player.SetHiddenInTrashcan(true);
    }
}

void Trashcan::UpdateAnimation(void)
{
    if (animAttachIndex_ == -1)
    {
        return;
    }

    switch (state_)
    {
    case TrashcanState::Opening:
        animTime_ += animSpeed_;

        if (animTime_ >= animTotalTime_)
        {
            animTime_ = animTotalTime_;
            state_ = TrashcanState::Open;
        }
        break;

    case TrashcanState::Closing:
        animTime_ -= animSpeed_;

        if (animTime_ <= 0.0f)
        {
            animTime_ = 0.0f;
            state_ = TrashcanState::Closed;
        }
        break;

    case TrashcanState::Closed:
    case TrashcanState::Open:
        break;
    }

    MV1SetAttachAnimTime(
        trans_.modelId,
        animAttachIndex_,
        animTime_
    );
}

void Trashcan::Draw(void)
{
    trans_.Update();

    MV1DrawModel(trans_.modelId);
}

bool Trashcan::CanInteract(const Player& player) const
{
    VECTOR playerPos = player.GetPos();

    VECTOR trashPos = trans_.pos;

    VECTOR diff = VSub(playerPos, trashPos);
    diff.y = 0.0f;

    float distance = VSize(diff);

    return distance <= interactDistance_;
}

bool Trashcan::DrawInteractUI(
    const Player& player
) const
{
    if (!CanInteract(player))
    {
        return false;
    }

    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    // =========================
    // UIの大きさと位置
    // =========================

    const int boxW = 420;
    const int boxH = 82;

    const int boxX =
        screenW / 2 - boxW / 2;

    const int boxY =
        screenH / 2 - boxH / 2;

    // =========================
    // ゴミ箱の状態で文字を切り替える
    // =========================

    const char* actionText = "";

    switch (state_)
    {
    case TrashcanState::Closed:
    case TrashcanState::Closing:
        actionText = "開ける";
        break;

    case TrashcanState::Open:
    case TrashcanState::Opening:
        actionText = "閉める";
        break;
    }

    // =========================
    // 枠の点滅
    // =========================

    const float time =
        static_cast<float>(
            GetNowCount()
            ) / 1000.0f;

    const float blink =
        (sinf(time * 5.0f) + 1.0f)
        * 0.5f;

    const int borderAlpha =
        150
        + static_cast<int>(
            blink * 90.0f
            );

    // =========================
    // 背景
    // =========================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        190
    );

    DrawBox(
        boxX,
        boxY,
        boxX + boxW,
        boxY + boxH,
        GetColor(5, 10, 18),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 外枠
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        borderAlpha
    );

    DrawBox(
        boxX,
        boxY,
        boxX + boxW,
        boxY + boxH,
        GetColor(120, 220, 255),
        FALSE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 内側の枠
    DrawBox(
        boxX + 4,
        boxY + 4,
        boxX + boxW - 4,
        boxY + boxH - 4,
        GetColor(35, 80, 100),
        FALSE
    );

    // =========================
    // F / X の表示情報
    // =========================

    const char* keyboardText = "F";
    const char* separatorText = "/";
    const char* padText = "X";

    const int keyboardTextW =
        GetDrawStringWidth(
            keyboardText,
            static_cast<int>(
                strlen(keyboardText)
                )
        );

    const int separatorTextW =
        GetDrawStringWidth(
            separatorText,
            static_cast<int>(
                strlen(separatorText)
                )
        );

    const int padTextW =
        GetDrawStringWidth(
            padText,
            static_cast<int>(
                strlen(padText)
                )
        );

    const int actionTextW =
        GetDrawStringWidth(
            actionText,
            static_cast<int>(
                strlen(actionText)
                )
        );

    // =========================
    // 各部品のサイズ
    // =========================

    const int keyBoxW = 46;
    const int keyBoxH = 46;

    const int padRadius = 21;
    const int padDiameter =
        padRadius * 2;

    const int partMargin = 10;
    const int actionMargin = 18;

    const int totalInputW =
        keyBoxW
        + partMargin
        + separatorTextW
        + partMargin
        + padDiameter
        + actionMargin
        + actionTextW;

    // 入力表示全体を中央揃え
    const int inputStartX =
        boxX
        + boxW / 2
        - totalInputW / 2;

    const int inputY =
        boxY
        + boxH / 2
        - keyBoxH / 2;

    const int whiteColor =
        GetColor(255, 255, 255);

    const int xButtonColor =
        GetColor(70, 160, 255);

    // =========================
    // Fキーの四角
    // =========================

    const int keyBoxX =
        inputStartX;

    const int keyBoxY =
        inputY;

    DrawBox(
        keyBoxX,
        keyBoxY,
        keyBoxX + keyBoxW,
        keyBoxY + keyBoxH,
        whiteColor,
        FALSE
    );

    // 内側の枠
    DrawBox(
        keyBoxX + 3,
        keyBoxY + 3,
        keyBoxX + keyBoxW - 3,
        keyBoxY + keyBoxH - 3,
        GetColor(100, 100, 110),
        FALSE
    );

    const int keyboardTextX =
        keyBoxX
        + keyBoxW / 2
        - keyboardTextW / 2;

    const int keyboardTextY =
        keyBoxY
        + keyBoxH / 2
        - 8;

    DrawString(
        keyboardTextX,
        keyboardTextY,
        keyboardText,
        whiteColor
    );

    // =========================
    // 区切りの「/」
    // =========================

    const int separatorX =
        keyBoxX
        + keyBoxW
        + partMargin;

    const int separatorY =
        keyBoxY
        + keyBoxH / 2
        - 8;

    DrawString(
        separatorX,
        separatorY,
        separatorText,
        GetColor(200, 200, 200)
    );

    // =========================
    // Xボタンの丸
    // =========================

    const int padCenterX =
        separatorX
        + separatorTextW
        + partMargin
        + padRadius;

    const int padCenterY =
        keyBoxY
        + keyBoxH / 2;

    // 外側の青い丸
    DrawCircle(
        padCenterX,
        padCenterY,
        padRadius,
        xButtonColor,
        FALSE
    );

    // 内側の丸
    DrawCircle(
        padCenterX,
        padCenterY,
        padRadius - 3,
        GetColor(35, 90, 150),
        FALSE
    );

    // Xを中央へ配置
    const int padTextX =
        padCenterX
        - padTextW / 2;

    const int padTextY =
        padCenterY - 8;

    DrawString(
        padTextX,
        padTextY,
        padText,
        xButtonColor
    );

    // =========================
    // 「開ける」「閉める」
    // =========================

    const int actionTextX =
        padCenterX
        + padRadius
        + actionMargin;

    const int actionTextY =
        padCenterY - 8;

    DrawString(
        actionTextX,
        actionTextY,
        actionText,
        whiteColor
    );

    return true;
}

static void ResolvePlayerAABB(
    Player& player,
    const VECTOR& boxCenter,
    const VECTOR& boxHalf,
    float playerRadius
)
{
    VECTOR playerPos = player.GetPos();

    float minX = boxCenter.x - boxHalf.x;
    float maxX = boxCenter.x + boxHalf.x;

    float minZ = boxCenter.z - boxHalf.z;
    float maxZ = boxCenter.z + boxHalf.z;

    float minY = boxCenter.y - boxHalf.y;
    float maxY = boxCenter.y + boxHalf.y;

    float playerBottomY = playerPos.y;
    float playerTopY = playerPos.y + 150.0f;

    if (playerBottomY > maxY || playerTopY < minY)
    {
        return;
    }

    float closestX = playerPos.x;

    if (closestX < minX)
    {
        closestX = minX;
    }
    else if (closestX > maxX)
    {
        closestX = maxX;
    }

    float closestZ = playerPos.z;

    if (closestZ < minZ)
    {
        closestZ = minZ;
    }
    else if (closestZ > maxZ)
    {
        closestZ = maxZ;
    }

    float diffX = playerPos.x - closestX;
    float diffZ = playerPos.z - closestZ;

    float distSq =
        diffX * diffX +
        diffZ * diffZ;

    if (distSq > playerRadius * playerRadius)
    {
        return;
    }

    float dist = sqrtf(distSq);

    if (dist < 0.001f)
    {
        float pushLeft = fabsf(playerPos.x - minX);
        float pushRight = fabsf(maxX - playerPos.x);
        float pushFront = fabsf(playerPos.z - minZ);
        float pushBack = fabsf(maxZ - playerPos.z);

        float minPush = pushLeft;
        VECTOR pushDir = VGet(-1.0f, 0.0f, 0.0f);

        if (pushRight < minPush)
        {
            minPush = pushRight;
            pushDir = VGet(1.0f, 0.0f, 0.0f);
        }

        if (pushFront < minPush)
        {
            minPush = pushFront;
            pushDir = VGet(0.0f, 0.0f, -1.0f);
        }

        if (pushBack < minPush)
        {
            minPush = pushBack;
            pushDir = VGet(0.0f, 0.0f, 1.0f);
        }

        player.SetPos(
            VAdd(
                playerPos,
                VScale(pushDir, playerRadius)
            )
        );

        return;
    }

    float push = playerRadius - dist;

    VECTOR pushDir =
        VGet(
            diffX / dist,
            0.0f,
            diffZ / dist
        );

    player.SetPos(
        VAdd(
            playerPos,
            VScale(pushDir, push)
        )
    );
}

void Trashcan::CheckPlayerCollision(Player& player)
{
    VECTOR outerCenter =
        VAdd(
            trans_.pos,
            outerOffset_
        );

    VECTOR innerCenter =
        VAdd(
            trans_.pos,
            innerOffset_
        );

    // 壁の厚み
    float wallX =
        outerHalf_.x - innerHalf_.x;

    float wallZ =
        outerHalf_.z - innerHalf_.z;

    if (wallX < 10.0f)
    {
        wallX = 10.0f;
    }

    if (wallZ < 10.0f)
    {
        wallZ = 10.0f;
    }

    // =========================
    // 左壁
    // =========================
    VECTOR leftWallCenter =
        VGet(
            outerCenter.x - innerHalf_.x - wallX * 0.5f,
            outerCenter.y,
            outerCenter.z
        );

    VECTOR leftWallHalf =
        VGet(
            wallX * 0.5f,
            outerHalf_.y,
            outerHalf_.z
        );

    // =========================
    // 右壁
    // =========================
    VECTOR rightWallCenter =
        VGet(
            outerCenter.x + innerHalf_.x + wallX * 0.5f,
            outerCenter.y,
            outerCenter.z
        );

    VECTOR rightWallHalf =
        VGet(
            wallX * 0.5f,
            outerHalf_.y,
            outerHalf_.z
        );

    // =========================
    // 奥壁
    // 今回は +Z 側を奥として扱う
    // =========================
    VECTOR backWallCenter =
        VGet(
            outerCenter.x,
            outerCenter.y,
            outerCenter.z + innerHalf_.z + wallZ * 0.5f
        );

    VECTOR backWallHalf =
        VGet(
            innerHalf_.x,
            outerHalf_.y,
            wallZ * 0.5f
        );

    // =========================
    // 前壁
    // 閉じている時だけ当たり判定あり
    // 開いている時はここを通って中に入れる
    // =========================
    bool isPlayerInside =
        IsPlayerInsideInner(player);

    bool frontWallActive =
        (state_ == TrashcanState::Closed ||
            state_ == TrashcanState::Closing) &&
        !isPlayerInside;
    VECTOR frontWallCenter =
        VGet(
            outerCenter.x,
            outerCenter.y,
            outerCenter.z - innerHalf_.z - wallZ * 0.5f
        );

    VECTOR frontWallHalf =
        VGet(
            innerHalf_.x,
            outerHalf_.y,
            wallZ * 0.5f
        );

    ResolvePlayerAABB(
        player,
        leftWallCenter,
        leftWallHalf,
        playerRadius_
    );

    ResolvePlayerAABB(
        player,
        rightWallCenter,
        rightWallHalf,
        playerRadius_
    );

    ResolvePlayerAABB(
        player,
        backWallCenter,
        backWallHalf,
        playerRadius_
    );

    if (frontWallActive)
    {
        ResolvePlayerAABB(
            player,
            frontWallCenter,
            frontWallHalf,
            playerRadius_
        );
    }
}


#ifdef _DEBUG
static void DrawDebugBox3D(
    const VECTOR& center,
    const VECTOR& half,
    int color
)
{
    float minX = center.x - half.x;
    float maxX = center.x + half.x;

    float minY = center.y - half.y;
    float maxY = center.y + half.y;

    float minZ = center.z - half.z;
    float maxZ = center.z + half.z;

    VECTOR p000 = VGet(minX, minY, minZ);
    VECTOR p100 = VGet(maxX, minY, minZ);
    VECTOR p110 = VGet(maxX, minY, maxZ);
    VECTOR p010 = VGet(minX, minY, maxZ);

    VECTOR p001 = VGet(minX, maxY, minZ);
    VECTOR p101 = VGet(maxX, maxY, minZ);
    VECTOR p111 = VGet(maxX, maxY, maxZ);
    VECTOR p011 = VGet(minX, maxY, maxZ);

    DrawLine3D(p000, p100, color);
    DrawLine3D(p100, p110, color);
    DrawLine3D(p110, p010, color);
    DrawLine3D(p010, p000, color);

    DrawLine3D(p001, p101, color);
    DrawLine3D(p101, p111, color);
    DrawLine3D(p111, p011, color);
    DrawLine3D(p011, p001, color);

    DrawLine3D(p000, p001, color);
    DrawLine3D(p100, p101, color);
    DrawLine3D(p110, p111, color);
    DrawLine3D(p010, p011, color);
}

void Trashcan::DebugDrawCollision(void) const
{
    SetUseLighting(FALSE);
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);

    VECTOR outerCenter =
        VAdd(
            trans_.pos,
            outerOffset_
        );

    VECTOR innerCenter =
        VAdd(
            trans_.pos,
            innerOffset_
        );

    float wallX =
        outerHalf_.x - innerHalf_.x;

    float wallZ =
        outerHalf_.z - innerHalf_.z;

    if (wallX < 10.0f)
    {
        wallX = 10.0f;
    }

    if (wallZ < 10.0f)
    {
        wallZ = 10.0f;
    }

    VECTOR leftWallCenter =
        VGet(
            outerCenter.x - innerHalf_.x - wallX * 0.5f,
            outerCenter.y,
            outerCenter.z
        );

    VECTOR leftWallHalf =
        VGet(
            wallX * 0.5f,
            outerHalf_.y,
            outerHalf_.z
        );

    VECTOR rightWallCenter =
        VGet(
            outerCenter.x + innerHalf_.x + wallX * 0.5f,
            outerCenter.y,
            outerCenter.z
        );

    VECTOR rightWallHalf =
        VGet(
            wallX * 0.5f,
            outerHalf_.y,
            outerHalf_.z
        );

    VECTOR backWallCenter =
        VGet(
            outerCenter.x,
            outerCenter.y,
            outerCenter.z + innerHalf_.z + wallZ * 0.5f
        );

    VECTOR backWallHalf =
        VGet(
            innerHalf_.x,
            outerHalf_.y,
            wallZ * 0.5f
        );

    VECTOR frontWallCenter =
        VGet(
            outerCenter.x,
            outerCenter.y,
            outerCenter.z - innerHalf_.z - wallZ * 0.5f
        );

    VECTOR frontWallHalf =
        VGet(
            innerHalf_.x,
            outerHalf_.y,
            wallZ * 0.5f
        );

    // 外側全体：紫
    DrawDebugBox3D(
        outerCenter,
        outerHalf_,
        GetColor(255, 0, 255)
    );

    // 中に入れる空洞：水色
    DrawDebugBox3D(
        innerCenter,
        innerHalf_,
        GetColor(0, 255, 255)
    );

    // 実際に当たり判定がある壁：黄色
    DrawDebugBox3D(
        leftWallCenter,
        leftWallHalf,
        GetColor(255, 255, 0)
    );

    DrawDebugBox3D(
        rightWallCenter,
        rightWallHalf,
        GetColor(255, 255, 0)
    );

    DrawDebugBox3D(
        backWallCenter,
        backWallHalf,
        GetColor(255, 255, 0)
    );

    if (state_ == TrashcanState::Closed ||
        state_ == TrashcanState::Closing)
    {
        DrawDebugBox3D(
            frontWallCenter,
            frontWallHalf,
            GetColor(255, 80, 80)
        );
    }

    SetWriteZBuffer3D(TRUE);
    SetUseZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}
#endif

bool Trashcan::IsClosedState(void) const
{
    return state_ == TrashcanState::Closed ||
        state_ == TrashcanState::Closing;
}

bool Trashcan::IsPlayerInsideInner(const Player& player) const
{
    VECTOR playerPos = player.GetPos();

    VECTOR innerCenter =
        VAdd(
            trans_.pos,
            innerOffset_
        );

    float innerMinX = innerCenter.x - innerHalf_.x;
    float innerMaxX = innerCenter.x + innerHalf_.x;

    float innerMinY = innerCenter.y - innerHalf_.y;
    float innerMaxY = innerCenter.y + innerHalf_.y;

    float innerMinZ = innerCenter.z - innerHalf_.z;
    float innerMaxZ = innerCenter.z + innerHalf_.z;

    float playerBottomY = playerPos.y;
    float playerTopY = playerPos.y + 150.0f;

    return
        playerPos.x >= innerMinX &&
        playerPos.x <= innerMaxX &&
        playerPos.z >= innerMinZ &&
        playerPos.z <= innerMaxZ &&
        playerBottomY <= innerMaxY &&
        playerTopY >= innerMinY;
}