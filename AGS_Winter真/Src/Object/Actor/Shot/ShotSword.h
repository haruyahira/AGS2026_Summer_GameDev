#pragma once
#include <vector>
#include <DxLib.h>
#include "ShotBase.h"

// 剣型の弾
class ShotSword : public ShotBase
{
public:
    enum class SwordState
    {
        FLOAT,    // 円浮遊
        GATHER,   // プレイヤー真上集合
        ATTACK    // 敵に突撃
    };

    // 現在の状態
    SwordState state_ = SwordState::FLOAT;

    // コンストラクタ / デストラクタ
    ShotSword(TYPE type, int baseModelId);
    ~ShotSword(void);

    // 剣専用の浮遊モードON/OFF（A段階では「浮遊開始フラグ」として使用）
    void SetFloatMode(bool f) override
    {
        floatMode_ = f;
        floatTimer_ = 0.0f;
    }

protected:
    // コピーコンストラクタ（使わないけど一応）
    ShotSword(const ShotSword&) = default;

    // パラメータ設定
    void SetParam(void) override;
    // 移動処理
    void Move() override;
    // 描画処理（残像付き）
    void DrawModel() override;

private:
    // 浮遊フラグ
    bool  floatMode_ = false;
    float floatTimer_ = 0.0f;

    // 残像用
    std::vector<VECTOR> trailPos_;
    int trailMax_ = 15;   // 残像の最大数

    // 各フェーズ共通の基準速度
    float baseSpeed_ = 10.0f;

    // B：スロー演出用
    int attackFrame_ = 0; // ATTACK状態に入ってからの経過フレーム
};
