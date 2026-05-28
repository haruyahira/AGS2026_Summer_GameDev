#include "Hp.h"

Hp::Hp(void)
{
    maxHp_ = 1;
    currentHp_ = 1;

    invincibleTimer_ = 0.0f;
    invincibleFrame_ = 0.0f;
}

Hp::Hp(int maxHp, float invincibleFrame)
{
    Init(maxHp, invincibleFrame);
}

Hp::~Hp(void)
{
}

void Hp::Init(int maxHp, float invincibleFrame)
{
    maxHp_ = maxHp;
    currentHp_ = maxHp_;

    invincibleTimer_ = 0.0f;
    invincibleFrame_ = invincibleFrame;
}

void Hp::Update(void)
{
    if (invincibleTimer_ > 0.0f)
    {
        invincibleTimer_ -= 1.0f;
    }
}

bool Hp::Damage(int damage)
{
    if (IsDead())
    {
        return false;
    }

    if (IsInvincible())
    {
        return false;
    }

    currentHp_ -= damage;

    if (currentHp_ < 0)
    {
        currentHp_ = 0;
    }

    invincibleTimer_ = invincibleFrame_;

    return true;
}

void Hp::Heal(int heal)
{
    if (IsDead())
    {
        return;
    }

    currentHp_ += heal;

    if (currentHp_ > maxHp_)
    {
        currentHp_ = maxHp_;
    }
}

void Hp::Reset(void)
{
    currentHp_ = maxHp_;
    invincibleTimer_ = 0.0f;
}

bool Hp::IsDead(void) const
{
    return currentHp_ <= 0;
}

bool Hp::IsInvincible(void) const
{
    return invincibleTimer_ > 0.0f;
}

int Hp::GetCurrent(void) const
{
    return currentHp_;
}

int Hp::GetMax(void) const
{
    return maxHp_;
}

float Hp::GetRate(void) const
{
    if (maxHp_ <= 0)
    {
        return 0.0f;
    }

    return static_cast<float>(currentHp_) / static_cast<float>(maxHp_);
}