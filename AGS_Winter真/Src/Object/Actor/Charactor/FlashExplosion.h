#pragma once
#include <DxLib.h>

// ========================================
// 3Dリング描画関数（どこからでも使える）
// ========================================
static void DrawRing3D(const VECTOR& center, float radius, int div, unsigned int color)
{
    float step = DX_TWO_PI_F / div;

    for (int i = 0; i < div; i++)
    {
        float a1 = i * step;
        float a2 = (i + 1) * step;

        VECTOR p1 = VGet(
            center.x + cosf(a1) * radius,
            center.y,
            center.z + sinf(a1) * radius
        );

        VECTOR p2 = VGet(
            center.x + cosf(a2) * radius,
            center.y,
            center.z + sinf(a2) * radius
        );

        DrawLine3D(p1, p2, color);
    }
}

// ========================================
// 爆発（疑似）エフェクトクラス
// ========================================
class FlashExplosion
{
public:
    VECTOR pos;     // 発生位置
    int timer;      // 生存時間管理
    bool alive;

    FlashExplosion(VECTOR p)
    {
        pos = p;
        timer = 0;
        alive = true;
    }

    void Update()
    {
        timer++;
        if (timer > 50) alive = false;
    }

    void Draw()
    {
        if (!alive) return;

        int t = timer;

        // --- ① 画面フラッシュ（最初の5フレーム） ---
        if (t < 5)
        {
            int alpha = 255 - t * 50;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(0, 0, 1920, 1080, GetColor(255, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // --- ② 光のリング（3D） ---
        if (t < 30)
        {
            float r = t * 12.0f;
            int alpha = 200 - t * 6;
            unsigned int col = GetColor(255, 255, 200);

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawRing3D(pos, r, 48, col);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // --- ③ デブリ（光の粒が飛び散る） ---
        for (int i = 0; i < 12; i++)
        {
            float ang = DX_TWO_PI_F * i / 12.0f;
            float dist = t * 4.0f;

            int alpha = 255 - t * 8;
            if (alpha < 0) alpha = 0;

            VECTOR p = VGet(
                pos.x + cosf(ang) * dist,
                pos.y + sinf(ang * 2.0f) * 5.0f,
                pos.z + sinf(ang) * dist
            );

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawSphere3D(p, 8.0f, 8, GetColor(255, 200, 120), GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // --- ④ 爆光（中心で光る球） ---
        {
            int alpha = 200 - t * 4;
            if (alpha < 0) alpha = 0;

            float scale = 40.0f + sinf(t * 0.3f) * 10.0f + t * 2.0f;

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawSphere3D(pos, scale, 16, GetColor(255, 255, 200), GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
};
