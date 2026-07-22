#pragma once

#include <DxLib.h>
#include "Furniture.h"

class TV : public Furniture
{
public:
    TV(const Transform* trans);
    ~TV() override;

    void Init() override;
    void Update() override;
    void Draw() override;

    void Play();
    void Stop();

private:
    void UpdateMovieTexture();
    void Update3DSoundPosition();
    void RestartMovieAndSound();

private:
    // =========================
    // 動画
    // =========================

    int movieHandle_ = -1;
    int movieScreen_ = -1;

    int screenTextureIndex_ = -1;

    int movieWidth_ = 0;
    int movieHeight_ = 0;

    bool isPlaying_ = false;

    // 画面内の映像回転
    float movieRotation_ =
        DX_PI_F / 2.0f;

    // 映像の大きさ
    float movieScaleAdjust_ =
        1.0f;

    // 映像の表示位置
    int movieOffsetX_ = -256;
    int movieOffsetY_ = 0;

    // 中間テクスチャサイズ
    static constexpr int MOVIE_TEXTURE_WIDTH = 1024;
    static constexpr int MOVIE_TEXTURE_HEIGHT = 1024;

    // =========================
    // 3Dサウンド
    // =========================

    int soundHandle_ = -1;

    VECTOR soundPosition_ =
        VGet(0.0f, 0.0f, 0.0f);

    // 音源位置の高さ調整
    float soundOffsetY_ = 80.0f;

    // 音が聞こえる範囲
    float soundRadius_ = 1500.0f;

    // 0から255
    int soundVolume_ = 255;
};