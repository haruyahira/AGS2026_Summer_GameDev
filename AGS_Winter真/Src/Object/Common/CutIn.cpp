#include "CutIn.h"

void CutIn::Init()
{
    imgGlow_ = LoadGraph("Data/CutIn/glow.png");
    imgShadow_ = LoadGraph("Data/CutIn/shadow.png");
    imgSpeedLine_ = LoadGraph("Data/CutIn/speedline.png");
    imgCutin_ = LoadGraph("Data/CutIn/pose.png");
    imgSkillName_ = LoadGraph("Data/CutIn/skillname.png");
}

void CutIn::Start()
{
    active_ = true;
    timer_ = 0;
}

void CutIn::Update()
{
    if (!active_) return;

    timer_++;

    if (timer_ > 90)
        active_ = false;
}

void CutIn::Draw()
{
    if (!active_) return;

    int t = timer_;

    // ‡@ ‰æ–Ê”’ƒtƒ‰ƒbƒVƒ…
    if (t < 5)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 - t * 50);
        DrawBox(0, 0, 1920, 1080, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ‡A ”wŒiˆÃ“]
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ‡B ‰~Œ`ƒOƒ[i–c’£j
    if (t > 5)
    {
        float s = (t - 5) * 0.04f;
        int size = (int)(800 * s);

        SetDrawBlendMode(DX_BLENDMODE_ADD, 150);
        DrawExtendGraph(
            960 - size, 540 - size,
            960 + size, 540 + size,
            imgGlow_, TRUE
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ‡C ƒLƒƒƒ‰ƒVƒ‹ƒGƒbƒg
    if (t > 10)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawGraph(460, 140, imgShadow_, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ‡D ƒXƒs[ƒhü
    if (t > 15)
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
        for (int i = 0; i < 8; i++)
        {
            int x = (t * 40 + i * 200) % 2000 - 300;
            DrawRotaGraph(x, 200 + i * 100, 1.3, -0.4, imgSpeedLine_, TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ‡E —§‚¿ŠGiƒXƒ‰ƒCƒhƒCƒ“j
    if (t > 20)
    {
        int slide = 1000 - (t - 20) * 40;
        if (slide < 0) slide = 0;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawGraph(slide, 0, imgCutin_, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ‡F ‹Z–¼ƒƒSiƒY[ƒ€j
    if (t > 35)
    {
        float s = 1.0f + (10 - abs(50 - t)) * 0.03f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawRotaGraph(960, 850, s, 0, imgSkillName_, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}
