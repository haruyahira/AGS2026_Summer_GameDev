#include <DxLib.h>
#include "../../Actor/ActorBase.h"
#include "../../../Manager/SceneManager.h"
#include "../../Actor/Charactor/Enemy.h"
#include "../../Actor/Charactor/Player.h"
#include "ShotSword.h"

ShotSword::ShotSword(TYPE type, int baseModelId)
    : ShotBase(type, baseModelId)
{
}

ShotSword::~ShotSword(void)
{
}

void ShotSword::SetParam(void)
{
    shotPos.clear();
    shotPos.push_back(pos_);

    // 剣の見た目サイズ
    scales_ = VGet(0.7f, 0.7f, 0.7f);

    // ベース速度
    baseSpeed_ = 10.0f;
    speed_ = baseSpeed_;

    // 生存関連
    isAlive_ = true;
    cntAlive_ = 300;
    collisionRadius_ = 30.0f;

    // ★ 発光強め（Aではちょい強化）
    MV1SetMaterialEmiColor(modelId_, 0, { 1.2f, 1.2f, 1.2f, 1.0f });

    // 状態初期化
    state_ = SwordState::FLOAT;
    floatMode_ = true;
    floatTimer_ = 0.0f;

    // 残像初期化
    trailPos_.clear();
    trailPos_.push_back(pos_);

    // スロー用フレームカウンタ
    attackFrame_ = 0;
}

void ShotSword::Move()
{
    // ─ 残像記録 ─
    trailPos_.push_back(pos_);
    if ((int)trailPos_.size() > trailMax_)
    {
        trailPos_.erase(trailPos_.begin());
    }

    Enemy* enemy = SceneManager::GetInstance().GetEnemy();
    Player* player = SceneManager::GetInstance().GetPlayer();
    VECTOR playerPos = player->GetTransform().GetPos();

    switch (state_)
    {
    case SwordState::FLOAT: // 円を描いて浮遊（A版では「その場ふわふわ」）
        floatTimer_ += 0.05f;
        pos_.y += sinf(floatTimer_ * 5.0f) * 0.5f;

        // 少し時間が経ったら集合フェーズへ
        if (floatTimer_ > 0.6f)
        {
            state_ = SwordState::GATHER;
            speed_ = baseSpeed_;
        }
        break;

    case SwordState::GATHER: // プレイヤー真上へ集合
    {
        VECTOR targetPos = playerPos;
        targetPos.y += 310.0f; // 少し高く

        VECTOR dir = VNorm(VSub(targetPos, pos_));
        pos_ = VAdd(pos_, VScale(dir, speed_ * 1.2f));

        // 近づいたら攻撃フェーズへ
        if (VSize(VSub(targetPos, pos_)) < 10.0f)
        {
            state_ = SwordState::ATTACK;
            speed_ = baseSpeed_;  // ATTACK では multiplier で調整する
            attackFrame_ = 0;     // ★ここからB：スロー演出★
        }
        break;
    }

    case SwordState::ATTACK: // 敵へ突撃（ここにスロー演出）
        if (!enemy)
        {
            isAlive_ = false;
            break;
        }
        else
        {
            VECTOR enemyPos = enemy->GetTransform().GetPos();
            VECTOR dirToEnemy = VNorm(VSub(enemyPos, pos_));

            // ── B：疑似スロー（0.25倍 → 通常 → 加速） ──
            attackFrame_++;

            float speedMul = 1.0f;

            // 0〜19フレーム：0.25倍（超スロー）
            if (attackFrame_ < 20)
            {
                speedMul = 0.25f;
            }
            // 20〜29フレーム：1.0倍（ちょい溜め終わり）
            else if (attackFrame_ < 30)
            {
                speedMul = 1.0f;
            }
            // 30フレーム以降：2.5倍（ドーンと加速）
            else
            {
                speedMul = 2.5f;
            }

            pos_ = VAdd(pos_, VScale(dirToEnemy, baseSpeed_ * speedMul));

            // 向きも敵方向へ
            float angleY = atan2f(dirToEnemy.x, dirToEnemy.z);
            MV1SetRotationXYZ(modelId_, VGet(0, angleY, DX_PI_F * 0.5f));
        }
        break;
    }

    // shotPos とモデル座標を同期
    if (!shotPos.empty())
        shotPos[0] = pos_;
    else
        shotPos.push_back(pos_);

    //MV1SetPosition(modelId_, pos_);
    //if (floatMode_)
    //{
    //    floatTimer_ += 0.06f;

    //    // 螺旋回転
    //    float angleSpeed = 4.0f;
    //    float a = angleSpeed * floatTimer_;
    //    pos_.x += cosf(a) * 0.8f;
    //    pos_.z += sinf(a) * 0.8f;

    //    // 上下ふわふわ
    //    pos_.y += sinf(floatTimer_ * 4.0f) * 0.6f;

    //    if (!shotPos.empty())
    //        shotPos[0] = pos_;

    //    MV1SetPosition(modelId_, pos_);
    //    return;
    //}

    //// ===== ここから突撃フェーズ =====
    //Enemy* enemy = SceneManager::GetInstance().GetEnemy();
    //if (!enemy) { isAlive_ = false; return; }

    //VECTOR targetPos = enemy->GetTransform().GetPos();
    //VECTOR dirToEnemy = VNorm(VSub(targetPos, pos_));
    //pos_ = VAdd(pos_, VScale(dirToEnemy, speed_ * 2.2f));

    //if (!shotPos.empty())
    //    shotPos[0] = pos_;

    //MV1SetPosition(modelId_, pos_);
}

void ShotSword::DrawModel()
{
    // ── 残像描画 ──
    int n = (int)trailPos_.size();
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / (float)n;   // 0.0 ～ 1.0
        float alpha = t;                 // 後ろほど透明

        MV1SetOpacityRate(modelId_, alpha);

        // 向きは最後に設定されたものを流用（全残像同じ向きでOK）
        MV1SetPosition(modelId_, trailPos_[i]);
        MV1DrawModel(modelId_);
    }

    // ── 本体描画 ──
    MV1SetOpacityRate(modelId_, 1.0f);
    if (!shotPos.empty())
    {
        MV1SetPosition(modelId_, shotPos[0]);
    }
    MV1DrawModel(modelId_);
}
