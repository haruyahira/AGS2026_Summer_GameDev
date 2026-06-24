#pragma once

class HpUI
{
public:
    HpUI();
    ~HpUI();

    void DrawPlayerHP(const void* owner, int x, int y, const char* name);

private:
    void DrawHpRow(
        int x,
        int y,
        int width,
        int height,
        const char* name,
        int currentHp,
        int maxHp,
        float rate,
        bool isInvincible
    );

    float ClampRate(float rate);
};