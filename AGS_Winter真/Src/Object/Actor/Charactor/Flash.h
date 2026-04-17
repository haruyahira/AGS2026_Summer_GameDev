#pragma once
#include <DxLib.h>

class Flash
{
public:
    VECTOR pos;
    float size;
    float alpha;
    bool alive;

    Flash(VECTOR p)
        : pos(p), size(1.0f), alpha(1.0f), alive(true)
    {
    }

    void Update()
    {
        size += 8.0f;      // 拡大スピード
        alpha -= 0.06f;    // 消えるスピード

        if (alpha <= 0.0f)
            alive = false;
    }

    void Draw()
    {
        if (!alive) return;

        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(alpha * 255));

        DrawBillboard3D(
            pos,                 // 中心
            size, size,          // 大きさ
            0.0f,
            0,
            GetColor(255, 255, 255),   // 白い光
            TRUE                       // 加算
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
};
