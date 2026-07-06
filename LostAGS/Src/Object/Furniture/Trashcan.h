#pragma once

#include "Furniture.h"

class Player;

enum class TrashcanState
{
    Closed,
    Opening,
    Open,
    Closing
};

class Trashcan : public Furniture
{
public:
    Trashcan(const Transform* trans);

    void Init(void) override;
    void Update(void) override;
    void Update(Player& player);
    void Draw(void) override;

    bool CanInteract(const Player& player) const;
    bool DrawInteractUI(const Player& player) const;

#ifdef _DEBUG
    void DebugDrawCollision(void) const;
#endif

private:
    void UpdateAnimation(void);
    void CheckPlayerCollision(Player& player);

private:
    int animAttachIndex_ = -1;
    float animTotalTime_ = 0.0f;
    float animTime_ = 0.0f;
    float animSpeed_ = 0.5f;

    TrashcanState state_ = TrashcanState::Closed;

    // ƒSƒ~” ‘S‘Ì
    VECTOR outerHalf_;
    VECTOR outerOffset_;

    // ’†‚É“ü‚ê‚é‹ó“´
    VECTOR innerHalf_;
    VECTOR innerOffset_;

    float playerRadius_ = 35.0f;
    float interactDistance_ = 150.0f;

    bool IsPlayerInsideInner(const Player& player) const;
    bool IsClosedState(void) const;
};