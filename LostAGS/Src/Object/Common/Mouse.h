#pragma once
#include <DxLib.h>

class Mouse {
public:
    Mouse();
    ~Mouse() = default;

    void Update(); // 毎フレーム更新
    void DrawDebug(); // 座標などを画面に表示（デバッグ用）

    // 座標取得
    int GetX() const { return x_; }
    int GetY() const { return y_; }

    // 前フレームからの移動量（視点回転に使用）
    int GetDiffX() const { return diffX_; }
    int GetDiffY() const { return diffY_; }

    // ボタン判定
    bool IsPress(int button) const; // 押しっぱなし
    bool IsTrg(int button) const;   // 押した瞬間

private:
    int x_, y_;           // 現在の座標
    int diffX_, diffY_;   // 移動量
    int mouseInput_;      // 現在の入力状態
    int prevMouseInput_;  // 前フレームの入力状態
};