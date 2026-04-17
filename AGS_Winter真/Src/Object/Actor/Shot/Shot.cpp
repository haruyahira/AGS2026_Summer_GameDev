#include <DxLib.h>
#include "../../Actor/ActorBase.h"
#include "Shot.h"

Shot::Shot(TYPE type, int baseModelId)
    : ShotBase(type, baseModelId)
{
}

Shot::~Shot(void)
{
}

void Shot::SetParam(void)
{
    // ★ここでは shotPos.push_back(pos_) は絶対にしない！
    //   （ShotBase::CreateShot ですでに 1 回 push 済み）

    // 弾の大きさ（※今回はモデル使わないので適当でOK）
    scales_ = { 0.4f, 0.4f, 0.4f };

    // 弾の速度
    speed_ = 6.0f;

    // 生存フラグと寿命
    isAlive_ = true;
    cntAlive_ = 300;      // 300フレーム（約5秒）

    // 衝突判定の半径（敵に当てたい範囲）
    collisionRadius_ = 30.0f;
    if (collider_)
    {
        collider_->SetRadius(collisionRadius_);
    }
}

// ★ここが「周りが光っている魔法弾」の本体
void Shot::DrawModel(void)
{
    if (shotPos.empty()) return;

    VECTOR p = shotPos[0];

    // 時間で揺れる値（脈動・回転用）
    static float t = 0.0f;
    t += 0.2f;

    // ========== コア部分 ==========
    float coreR = 12.0f + sinf(t * 1.5f) * 2.0f;  // コアが揺れる

    DrawSphere3D(
        p,
        coreR,
        16,
        GetColor(255, 255, 150),  // 明るい黄色
        GetColor(255, 200, 80),   // 外側少しオレンジ寄り
        TRUE
    );

    // ========== 周囲の光のオーラ（脈動） ==========
    float glowBase = 28.0f;
    float glowPulse = 4.0f * sinf(t * 2.0f);   // 脈動
    float glowR = glowBase + glowPulse;

    int oldBlendMode, oldBlendParam;
    GetDrawBlendMode(&oldBlendMode, &oldBlendParam);

    SetDrawBlendMode(DX_BLENDMODE_ADD, 160);

    DrawSphere3D(
        p,
        glowR,
        16,
        GetColor(255, 255, 100),  // 黄色いオーラ
        GetColor(0, 0, 0),
        TRUE
    );

    // ========== 回転リング ==========

    SetDrawBlendMode(DX_BLENDMODE_ADD, 200);

    const int RING_NUM = 2;
    for (int i = 0; i < RING_NUM; i++)
    {
        float angle = t * (1.2f + i * 0.5f); // 少し速度差をつける
        float ringR = 20.0f;                 // リング半径

        // リング上の点を複数線で描く
        const int SEG = 36;
        for (int j = 0; j < SEG; j++)
        {
            float a1 = angle + DX_TWO_PI_F * (j / (float)SEG);
            float a2 = angle + DX_TWO_PI_F * ((j + 1) / (float)SEG);

            VECTOR p1 = VAdd(p, VGet(cosf(a1) * ringR, sinf(a1) * 2.0f, sinf(a1) * ringR));
            VECTOR p2 = VAdd(p, VGet(cosf(a2) * ringR, sinf(a2) * 2.0f, sinf(a2) * ringR));

            DrawLine3D(p1, p2, GetColor(255, 230, 80)); // 黄色リング
        }
    }

    // ブレンドを戻す
    SetDrawBlendMode(oldBlendMode, oldBlendParam);
}

