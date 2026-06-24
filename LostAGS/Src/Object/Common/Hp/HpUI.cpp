#include "HpUI.h"
#include "HPManager.h"
#include "Hp.h"
#include "DxLib.h"

HpUI::HpUI()
{
}

HpUI::~HpUI()
{
}

void HpUI::DrawPlayerHP(const void* owner, int x, int y, const char* name)
{
    const Hp* hp = HpManager::GetInstance().GetHP(owner);

    if (hp == nullptr)
    {
        return;
    }

    int currentHp = hp->GetCurrent();
    int maxHp = hp->GetMax();
    float rate = hp->GetRate();
    bool isInvincible = hp->IsInvincible();

    DrawHpRow(
        x,
        y,
        480,
        60,
        name,
        currentHp,
        maxHp,
        rate,
        isInvincible
    );
}

void HpUI::DrawHpRow(
    int x,
    int y,
    int width,
    int height,
    const char* name,
    int currentHp,
    int maxHp,
    float rate,
    bool isInvincible
)
{
    rate = ClampRate(rate);

    int white = GetColor(255, 255, 255);
    int black = GetColor(0, 0, 0);
    int darkGray = GetColor(35, 35, 35);
    int panelGray = GetColor(190, 190, 190);
    int hpGreen = GetColor(90, 220, 50);
    int hpYellow = GetColor(230, 210, 40);
    int hpRed = GetColor(230, 50, 70);
    int shadow = GetColor(10, 10, 10);

    // 無敵中は少し明るい色にする
    int hpColor = hpGreen;

    if (rate <= 0.25f)
    {
        hpColor = hpRed;
    }
    else if (rate <= 0.5f)
    {
        hpColor = hpYellow;
    }

    if (isInvincible)
    {
        hpColor = GetColor(255, 255, 255);
    }

    // =========================
    // 背景パネル
    // =========================

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 135);
    DrawBox(x, y, x + width, y + height, darkGray, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 95);
    DrawBox(x, y, x + width, y + height, panelGray, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


    // =========================
    // 名前
    // =========================

    int nameX = x + 40;
    int nameY = y + 22;

    DrawString(nameX + 2, nameY + 2, name, shadow);
    DrawString(nameX, nameY, name, white);

    // =========================
    // HPバー外枠
    // =========================

    int barX = x + 130;
    int barY = y + 17;
    int barW = width - 150;
    int barH = 28;

    DrawBox(barX - 2, barY - 2, barX + barW + 2, barY + barH + 2, black, TRUE);
    DrawBox(barX, barY, barX + barW, barY + barH, white, FALSE);

    // バー背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
    DrawBox(barX + 3, barY + 3, barX + barW - 3, barY + barH - 3, GetColor(40, 40, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // =========================
    // HP残量
    // =========================

    int innerX = barX + 5;
    int innerY = barY + 5;
    int innerW = barW - 10;
    int innerH = barH - 10;

    int fillW = static_cast<int>(innerW * rate);

    if (fillW > 0)
    {
        int slant = 18;

        if (fillW > slant)
        {
            // 右端が斜めになっているHPバー
            DrawQuadrangle(
                innerX,
                innerY,
                innerX + fillW,
                innerY,
                innerX + fillW - slant,
                innerY + innerH,
                innerX,
                innerY + innerH,
                hpColor,
                TRUE
            );
        }
        else
        {
            DrawBox(
                innerX,
                innerY,
                innerX + fillW,
                innerY + innerH,
                hpColor,
                TRUE
            );
        }
    }

    // =========================
    // HP数値
    // =========================

    DrawFormatString(
        barX + barW - 65,
        barY + 28,
        white,
        "%d / %d",
        currentHp,
        maxHp
    );
}

float HpUI::ClampRate(float rate)
{
    if (rate < 0.0f)
    {
        return 0.0f;
    }

    if (rate > 1.0f)
    {
        return 1.0f;
    }

    return rate;
}