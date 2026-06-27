#include "HPManager.h"

HpManager& HpManager::GetInstance(void)
{
    static HpManager instance;
    return instance;
}

HpManager::HpManager(void)
{
}

HpManager::~HpManager(void)
{
    Clear();
}

void HpManager::RegisterHP(const void* owner, int maxHp, float invincibleFrame)
{
    if (owner == nullptr)
    {
        return;
    }

    hpMap_[owner] = std::make_unique<Hp>(maxHp, invincibleFrame);
}

void HpManager::UnregisterHP(const void* owner)
{
    if (owner == nullptr)
    {
        return;
    }

    hpMap_.erase(owner);
}

void HpManager::Update(void)
{
    for (auto& pair : hpMap_)
    {
        if (pair.second)
        {
            pair.second->Update();
        }
    }
}

void HpManager::Clear(void)
{
    hpMap_.clear();
}

bool HpManager::Damage(const void* owner, int damage)
{
    Hp* hp = GetHP(owner);

    if (hp == nullptr)
    {
        return false;
    }

    return hp->Damage(damage);
}

void HpManager::Heal(const void* owner, int heal)
{
    Hp* hp = GetHP(owner);

    if (hp == nullptr)
    {
        return;
    }

    hp->Heal(heal);
}

bool HpManager::IsDead(const void* owner) const
{
    const Hp* hp = GetHP(owner);

    if (hp == nullptr)
    {
        return false;
    }

    return hp->IsDead();
}

Hp* HpManager::GetHP(const void* owner)
{
    auto it = hpMap_.find(owner);

    if (it == hpMap_.end())
    {
        return nullptr;
    }

    return it->second.get();
}

const Hp* HpManager::GetHP(const void* owner) const
{
    auto it = hpMap_.find(owner);

    if (it == hpMap_.end())
    {
        return nullptr;
    }

    return it->second.get();
}