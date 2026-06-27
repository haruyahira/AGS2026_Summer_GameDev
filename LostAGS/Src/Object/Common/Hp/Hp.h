#pragma once

class Hp
{
public:
    Hp(void);
    Hp(int maxHp, float invincibleFrame = 0.0f);
    ~Hp(void);

    void Init(int maxHp, float invincibleFrame = 0.0f);
    void Update(void);

    bool Damage(int damage);
    void Heal(int heal);
    void Reset(void);

    bool IsDead(void) const;
    bool IsInvincible(void) const;

    int GetCurrent(void) const;
    int GetMax(void) const;
    float GetRate(void) const;

private:
    int maxHp_;
    int currentHp_;

    float invincibleTimer_;
    float invincibleFrame_;
};