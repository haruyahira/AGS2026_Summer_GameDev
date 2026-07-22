#include <cmath>
#include "TV.h"

// =========================
// コンストラクタ
// =========================

TV::TV(const Transform* trans)
    : Furniture(NAME::TV, trans)
{
}

// =========================
// デストラクタ
// =========================

TV::~TV()
{
    // モデルへ設定した差し替えテクスチャを解除
    if (trans_.modelId != -1 &&
        screenTextureIndex_ >= 0)
    {
        MV1SetTextureGraphHandle(
            trans_.modelId,
            screenTextureIndex_,
            -1,
            FALSE
        );
    }

    // 3Dサウンド解放
    if (soundHandle_ != -1)
    {
        StopSoundMem(
            soundHandle_
        );

        DeleteSoundMem(
            soundHandle_
        );

        soundHandle_ = -1;
    }

    // 動画解放
    if (movieHandle_ != -1)
    {
        DeleteGraph(
            movieHandle_
        );

        movieHandle_ = -1;
    }

    // 中間テクスチャ解放
    if (movieScreen_ != -1)
    {
        DeleteGraph(
            movieScreen_
        );

        movieScreen_ = -1;
    }
}

// =========================
// 初期化
// =========================

void TV::Init()
{
    // =========================
    // 動画の読み込み
    // =========================

    movieHandle_ =
        LoadGraph(
            "Data/Movie/TVMovie_NoSound.mp4"
        );

    if (movieHandle_ == -1)
    {
        printfDx(
            "TV movie load failed.\n"
        );

        return;
    }

    // 動画サイズを取得
    GetGraphSize(
        movieHandle_,
        &movieWidth_,
        &movieHeight_
    );

  /*  printfDx(
        "TV Movie Size = %d x %d\n",
        movieWidth_,
        movieHeight_
    );*/

    if (movieWidth_ <= 0 ||
        movieHeight_ <= 0)
    {
        printfDx(
            "TV movie size is invalid.\n"
        );

        DeleteGraph(
            movieHandle_
        );

        movieHandle_ = -1;

        return;
    }

    // =========================
    // 中間テクスチャ作成
    // =========================

    movieScreen_ =
        MakeScreen(
            MOVIE_TEXTURE_WIDTH,
            MOVIE_TEXTURE_HEIGHT,
            TRUE
        );

    if (movieScreen_ == -1)
    {
        printfDx(
            "TV movie screen creation failed.\n"
        );

        DeleteGraph(
            movieHandle_
        );

        movieHandle_ = -1;

        return;
    }

    // =========================
    // テレビ画面へ設定
    // =========================

    screenTextureIndex_ = 1;

    const int result =
        MV1SetTextureGraphHandle(
            trans_.modelId,
            screenTextureIndex_,
            movieScreen_,
            FALSE
        );

    if (result == -1)
    {
        printfDx(
            "TV texture setting failed.\n"
        );

        return;
    }

    // =========================
    // 3Dサウンドの読み込み
    // =========================

    // この後に読み込む音声を
    // 3Dサウンドとして作成する
    SetCreate3DSoundFlag(
        TRUE
    );

    soundHandle_ =
        LoadSoundMem(
            "Data/Movie/TVMovieSound.wav"
        );

    // 以降の音声へ影響しないように戻す
    SetCreate3DSoundFlag(
        FALSE
    );

    if (soundHandle_ == -1)
    {
        printfDx(
            "TV 3D sound load failed.\n"
        );
    }
    else
    {
        // 音源の有効半径
        Set3DRadiusSoundMem(
            soundRadius_,
            soundHandle_
        );

        // 音量
        ChangeVolumeSoundMem(
            soundVolume_,
            soundHandle_
        );

        Update3DSoundPosition();
    }

    // =========================
    // 映像と音声の再生開始
    // =========================

    PlayMovieToGraph(
        movieHandle_,
        DX_PLAYTYPE_BACK
    );

    if (soundHandle_ != -1)
    {
        PlaySoundMem(
            soundHandle_,
            DX_PLAYTYPE_BACK,
            TRUE
        );
    }

    isPlaying_ = true;

    // 最初の映像を作成
    UpdateMovieTexture();
}

// =========================
// 更新
// =========================

void TV::Update()
{
    // テレビが移動する場合にも対応
    Update3DSoundPosition();

    if (movieHandle_ == -1 ||
        movieScreen_ == -1)
    {
        return;
    }

    // 動画が終了したら、
    // 動画と音声を同時に最初から再生する
    if (isPlaying_ &&
        GetMovieStateToGraph(
            movieHandle_
        ) == 0)
    {
        RestartMovieAndSound();
    }

    // 動画の現在フレームを画面へ反映
    UpdateMovieTexture();
}

// =========================
// 3D音源位置更新
// =========================

void TV::Update3DSoundPosition()
{
    if (soundHandle_ == -1)
    {
        return;
    }

    soundPosition_ =
        trans_.pos;

    // テレビ画面付近の高さへ移動
    soundPosition_.y +=
        soundOffsetY_;

    Set3DPositionSoundMem(
        soundPosition_,
        soundHandle_
    );
}

// =========================
// 動画と音声を最初から再生
// =========================

void TV::RestartMovieAndSound()
{
    if (movieHandle_ != -1)
    {
        SeekMovieToGraph(
            movieHandle_,
            0
        );

        PlayMovieToGraph(
            movieHandle_,
            DX_PLAYTYPE_BACK
        );
    }

    if (soundHandle_ != -1)
    {
        StopSoundMem(
            soundHandle_
        );

        PlaySoundMem(
            soundHandle_,
            DX_PLAYTYPE_BACK,
            TRUE
        );
    }

    isPlaying_ = true;
}

// =========================
// 動画テクスチャ更新
// =========================

void TV::UpdateMovieTexture()
{
    if (movieHandle_ == -1 ||
        movieScreen_ == -1)
    {
        return;
    }

    if (movieWidth_ <= 0 ||
        movieHeight_ <= 0)
    {
        return;
    }

    const int textureWidth =
        MOVIE_TEXTURE_WIDTH;

    const int textureHeight =
        MOVIE_TEXTURE_HEIGHT;

    // 現在の描画先だけ保存する
    const int oldScreen =
        GetDrawScreen();

    // テレビ用スクリーンへ描画
    SetDrawScreen(
        movieScreen_
    );

    ClearDrawScreen();

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 背景を黒くする
    DrawBox(
        0,
        0,
        textureWidth,
        textureHeight,
        GetColor(0, 0, 0),
        TRUE
    );

    // =========================
    // 回転後の動画サイズを計算
    // =========================

    float rotatedWidth =
        static_cast<float>(
            movieWidth_
            );

    float rotatedHeight =
        static_cast<float>(
            movieHeight_
            );

    const float normalizedAngle =
        fmodf(
            fabsf(movieRotation_),
            DX_TWO_PI_F
        );

    // 90度または270度回転しているか
    const bool isQuarterTurn =
        normalizedAngle >
        DX_PI_F / 4.0f &&
        normalizedAngle <
        DX_PI_F * 3.0f / 4.0f;

    if (isQuarterTurn)
    {
        rotatedWidth =
            static_cast<float>(
                movieHeight_
                );

        rotatedHeight =
            static_cast<float>(
                movieWidth_
                );
    }

    // =========================
    // 中間スクリーンに収める倍率
    // =========================

    const float scaleX =
        static_cast<float>(
            textureWidth
            ) / rotatedWidth;

    const float scaleY =
        static_cast<float>(
            textureHeight
            ) / rotatedHeight;

    float scale =
        scaleX < scaleY
        ? scaleX
        : scaleY;

    scale *= movieScaleAdjust_;

    // =========================
    // 回転・位置調整して描画
    // =========================

    const int drawCenterX =
        textureWidth / 2 +
        movieOffsetX_;

    const int drawCenterY =
        textureHeight / 2 +
        movieOffsetY_;

    DrawRotaGraph(
        drawCenterX,
        drawCenterY,
        -scale,
        movieRotation_,
        movieHandle_,
        FALSE
    );

    // 元の描画先へ戻す
    SetDrawScreen(
        oldScreen
    );

    // 標準のブレンド状態へ戻す
    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

// =========================
// 描画
// =========================

void TV::Draw()
{
    trans_.Update();

    MV1DrawModel(
        trans_.modelId
    );
}

// =========================
// 再生
// =========================

void TV::Play()
{
    if (isPlaying_)
    {
        return;
    }

    if (movieHandle_ != -1)
    {
        PauseMovieToGraph(
            movieHandle_,
            FALSE
        );
    }

    if (soundHandle_ != -1)
    {
        // 一時停止ではなく停止されていた場合は
        // 最初から再生する
        if (CheckSoundMem(
            soundHandle_
        ) == 0)
        {
            PlaySoundMem(
                soundHandle_,
                DX_PLAYTYPE_BACK,
                TRUE
            );
        }
    }

    isPlaying_ = true;
}

// =========================
// 停止
// =========================

void TV::Stop()
{
    if (!isPlaying_)
    {
        return;
    }

    if (movieHandle_ != -1)
    {
        PauseMovieToGraph(
            movieHandle_,
            TRUE
        );
    }

    if (soundHandle_ != -1)
    {
        StopSoundMem(
            soundHandle_
        );
    }

    isPlaying_ = false;
}