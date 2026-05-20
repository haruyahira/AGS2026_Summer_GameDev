#pragma once
#include <DxLib.h>

// 色をRGBAで保持する構造体
struct ColorRGBA {
    unsigned char r, g, b, a;

    // DXライブラリ用の unsigned int (GetColor値) に自動変換
    // これにより、Draw関数にそのまま放り込める
    operator unsigned int() const {
        return GetColor(r, g, b);
    };
};

namespace Color {
    // --- 汎用色 ---
    static const ColorRGBA WHITE = { 255, 255, 255, 255 };
    static const ColorRGBA BLACK = { 0,   0,   0, 255 };
    static const ColorRGBA GRAY = { 128, 128, 128, 255 };

    // --- タイトル画面用 ---
    namespace Title {
     // 文字の色

    }
}