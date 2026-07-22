#pragma once

#include <DxLib.h>
#include "../Furniture/Furniture.h"

class Player;

struct ShelfBoxCollision
{
    VECTOR center;
    VECTOR axisX;
    VECTOR axisZ;
    VECTOR half;
};

class ShelfBox : public Furniture
{
public:
    enum class DoorState
    {
        Closed,
        Opening,
        Open,
        Closing
    };

public:
    ShelfBox(const Transform* trans, float rotY, float hingeSide, float openSign);
    virtual ~ShelfBox() {}

    void Init() override;
    void Update(void) override;

    // プレイヤーを押し戻すため const を外す
    void Update(Player& player);

    void Draw(void) override;

    bool DrawInteractUI(const Player& player) const;
    void DebugDrawInteractRange(const Player& player) const;
    void DebugDrawCollision() const;
    void OpenByEnemy(void);
    bool IsPassableForEnemy(void) const;
    bool ResolveEnemyCollision(
        VECTOR& enemyPos,
        float enemyRadius,
        float bottomY,
        float topY
    ) const;
    bool IsEnemyNearDoor(
        const VECTOR& enemyPos,
        float enemyRadius,
        float bottomY,
        float topY,
        float margin
    ) const;

    bool IsOpen() const;


private:
    void UpdateAnimation();

    bool CanInteract(const Player& player) const;

    void UpdateCollision();
    void CheckPlayerCollision(Player& player);
    bool IsHitPlayer(const Player& player) const;




    DoorState state_ = DoorState::Closed;

    int animAttachIndex_ = -1;
    int animAttachIndex2_ = -1;
    float animTotalTime_ = 0.0f;
    float animTime_ = 0.0f;
    float animSpeed_ = 1.0f;

    // =========================
    // 扉用当たり判定
    // =========================

    ShelfBoxCollision collision_;

    VECTOR hingePos_ = VGet(0.0f, 0.0f, 0.0f);

    float closedRotY_ = 0.0f;
    float openAngle_ = 0.0f;

    float doorWidth_ = 520.0f;
    float doorHeight_ = 260.0f;
    float doorThickness_ = 40.0f;

    float playerRadius_ = 35.0f;

    bool isCollisionActive_ = true;

    float baseRotY_ = 0.0f;

    // 蝶番がどちら側にあるか
    // -1.0f : 片側
    //  1.0f : 反対側
    float hingeSide_ = -1.0f;

    // 開く向き
    //  1.0f : プラス方向に開く
    // -1.0f : マイナス方向に開く
    float openSign_ = 1.0f;
};