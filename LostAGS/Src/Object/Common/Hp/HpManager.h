#pragma once
#include <unordered_map>
#include <memory>
#include "Hp.h"

class HpManager
{
public:
    static HpManager& GetInstance(void);

    void RegisterHP(const void* owner, int maxHp, float invincibleFrame = 0.0f);
    void UnregisterHP(const void* owner);

    void Update(void);
    void Clear(void);

    bool Damage(const void* owner, int damage);
    void Heal(const void* owner, int heal);

    bool IsDead(const void* owner) const;

    Hp* GetHP(const void* owner);
    const Hp* GetHP(const void* owner) const;

private:
    HpManager(void);
    ~HpManager(void);

    HpManager(const HpManager&) = delete;
    HpManager& operator=(const HpManager&) = delete;

private:
    std::unordered_map<const void*, std::unique_ptr<Hp>> hpMap_;
};