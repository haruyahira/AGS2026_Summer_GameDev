#pragma once

#include <DxLib.h>
#include "Common/Transform.h"
#include "../Manager/ResourceManager.h"

class Player;

class Item
{
public:

    enum class TYPE
    {
        LAPTOP,     // ÉmÅ[ÉgPC
        WATCH,    // òréûåv
        KEY,        // åÆ
        MEDICINE,   // ñÚ

        MAX
    };

private:

    TYPE type_;
    Transform transform_;

    bool isActive_;

    float pickupRange_;
    float viewDot_;

    ResourceManager& resMng_;

public:

    Item();
    ~Item();

    void Init(TYPE type, ResourceManager::SRC modelSrc, VECTOR pos, VECTOR scl);
    void Update();
    void Draw();

    bool IsActive() const;
    void SetActive(bool active);

    TYPE GetType() const;
    const char* GetName() const;
    VECTOR GetPos() const;

    bool IsInPlayerView(const VECTOR& playerPos, const VECTOR& playerForward) const;
    void Pickup();
    float GetPickupRange(void) const;
};