#include <DxLib.h>
#include <cstring>

#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/SoundManager.h"
#include "GameClearScene.h"

GameClearScene::GameClearScene(void)
{
    clearImageHandle_ = -1;

    titleFontHandle_ = -1;
    roleFontHandle_ = -1;
    nameFontHandle_ = -1;
    guideFontHandle_ = -1;

    staffRollY_ = 0.0f;
    staffRollSpeed_ = 38.0f;

    startTime_ = 0;
    isInputEnabled_ = false;

    backgroundMoveTime_ = 0.0f;
    backgroundMoveSpeed_ = 0.35f;

    isStaffRollStopped_ = false;
}

GameClearScene::~GameClearScene(void)
{
    if (clearImageHandle_ != -1)
    {
        DeleteGraph(clearImageHandle_);
        clearImageHandle_ = -1;
    }

    if (titleFontHandle_ != -1)
    {
        DeleteFontToHandle(titleFontHandle_);
        titleFontHandle_ = -1;
    }

    if (roleFontHandle_ != -1)
    {
        DeleteFontToHandle(roleFontHandle_);
        roleFontHandle_ = -1;
    }

    if (nameFontHandle_ != -1)
    {
        DeleteFontToHandle(nameFontHandle_);
        nameFontHandle_ = -1;
    }

    if (guideFontHandle_ != -1)
    {
        DeleteFontToHandle(guideFontHandle_);
        guideFontHandle_ = -1;
    }
}

void GameClearScene::Init(void)
{
    SoundManager& snd =
        SoundManager::GetInstance();

    snd.SetBGMVolume(255);

    snd.PlayBGM(
        SoundManager::BGM::CLEAR,
        true
    );

    // 全画面表示用の一枚絵
    clearImageHandle_ = LoadGraph(
        "Data/Image/GameClear.png"
    );

    // GAME CLEAR、制作・著作など
    titleFontHandle_ = CreateFontToHandle(
        nullptr,
        72,
        5
    );

    // 役職、メッセージ
    roleFontHandle_ = CreateFontToHandle(
        nullptr,
        34,
        3
    );

    // スタッフ名
    nameFontHandle_ = CreateFontToHandle(
        nullptr,
        44,
        3
    );

    // 画面下部の案内
    guideFontHandle_ = CreateFontToHandle(
        nullptr,
        30,
        2
    );

    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    // 最初の文字を画面下から出現させる
    staffRollY_ =
        static_cast<float>(
            screenH + 100
            );

    // 大きな文字に合わせて少し速くする
    staffRollSpeed_ = 138.0f;

    backgroundMoveTime_ = 0.0f;
    backgroundMoveSpeed_ = 0.35f;

    startTime_ = GetNowCount();
    isInputEnabled_ = false;

    isStaffRollStopped_ = false;
}

void GameClearScene::Update(void)
{
    const float deltaTime =
        SceneManager::GetInstance().GetDeltaTime();

    backgroundMoveTime_ +=
        backgroundMoveSpeed_ * deltaTime;

    // 横移動と縦移動の両方が同じ位置に戻る周期
    const float backgroundLoopTime =
        DX_TWO_PI_F * 100.0f;

    if (backgroundMoveTime_ >= backgroundLoopTime)
    {
        backgroundMoveTime_ -= backgroundLoopTime;
    }

    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    /*
     * 現在のcredits配列における、
     * staffRollY_から
     * 「制作・著作」「しぼあま」の中央までの距離。
     */
     // staffRollY_から「しぼあま」の文字中央までの距離
    const float shiboamaCenterOffset =
        2716.0f;

    // スタッフロール停止前だけ上へ動かす
    if (!isStaffRollStopped_)
    {
        staffRollY_ -=
            staffRollSpeed_ * deltaTime;

        const float shiboamaCenterY =
            staffRollY_
            + shiboamaCenterOffset;

        const float screenCenterY =
            static_cast<float>(screenH) * 0.5f
            - 400.0f;

        /*
         * 「制作・著作」「しぼあま」の中央が
         * 画面中央へ到達したら、その位置で固定する。
         */
        if (shiboamaCenterY <= screenCenterY)
        {
            staffRollY_ =
                screenCenterY
                - shiboamaCenterOffset;

            isStaffRollStopped_ = true;
        }
    }

    // シーン開始から1秒後に操作可能
    if (GetNowCount() - startTime_ >= 1000)
    {
        isInputEnabled_ = true;
    }

    if (!isInputEnabled_)
    {
        return;
    }

    InputManager& ins =
        InputManager::GetInstance();

    const bool isDecision =
        ins.IsTrgDown(KEY_INPUT_RETURN) ||
        ins.IsTrgDown(KEY_INPUT_SPACE) ||
        ins.IsTrgDown(KEY_INPUT_F) ||
        ins.IsPadBtnTrgDown(
            InputManager::JOYPAD_NO::PAD1,
            InputManager::JOYPAD_BTN::DOWN
        );

    if (isDecision)
    {
        SoundManager::GetInstance()
            .StopAllSound();

        SceneManager::GetInstance().ResetGameResultData();

        SceneManager::GetInstance().ChangeScene(
            SceneManager::SCENE_ID::TITLE
        );

        return;
    }
}

void GameClearScene::Draw(void)
{
    // カスタムシェーダーを解除
    SetUseVertexShader(-1);
    SetUsePixelShader(-1);

    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);
    SetUseTextureToShader(2, -1);

    // 2D描画用の設定
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetUseBackCulling(FALSE);
    SetUseLighting(FALSE);

    SetDrawBright(
        255,
        255,
        255
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 描画順
    DrawBackground();
    DrawClearImage();
    DrawStaffRoll();
    DrawGuide();

    // 描画範囲を必ず全画面へ戻す
    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    SetDrawArea(
        0,
        0,
        screenW,
        screenH
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    SetDrawBright(
        255,
        255,
        255
    );
}

void GameClearScene::DrawBackground(void) const
{
    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    // 実際の描画画面全体を黒で塗る
    DrawBox(
        0,
        0,
        screenW,
        screenH,
        GetColor(0, 0, 0),
        TRUE
    );
}

void GameClearScene::DrawClearImage(void) const
{
    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    if (clearImageHandle_ != -1)
    {
        int imageW = 0;
        int imageH = 0;

        GetGraphSize(
            clearImageHandle_,
            &imageW,
            &imageH
        );

        if (imageW > 0 &&
            imageH > 0)
        {
            // 元画像の90パーセントを切り出す
            // 残り10パーセント分を移動領域として使用する
            const float cropRate = 0.90f;

            const int sourceW =
                static_cast<int>(
                    imageW * cropRate
                    );

            const int sourceH =
                static_cast<int>(
                    imageH * cropRate
                    );

            // 切り取り位置を移動できる範囲
            const int moveRangeX =
                imageW - sourceW;

            const int moveRangeY =
                imageH - sourceH;

            // 0.0から1.0の範囲で左右に往復
            const float moveXRate =
                (sinf(backgroundMoveTime_) + 1.0f)
                * 0.5f;

            // 横とは少し違う速度で上下に往復
            const float moveYRate =
                (cosf(backgroundMoveTime_ * 0.73f) + 1.0f)
                * 0.5f;

            const int sourceX =
                static_cast<int>(
                    moveRangeX * moveXRate
                    );

            const int sourceY =
                static_cast<int>(
                    moveRangeY * moveYRate
                    );

            // 移動する切り取り範囲を画面全体へ拡大表示
            DrawRectExtendGraph(
                0,
                0,
                screenW,
                screenH,
                sourceX,
                sourceY,
                sourceW,
                sourceH,
                clearImageHandle_,
                TRUE
            );
        }
        else
        {
            DrawExtendGraph(
                0,
                0,
                screenW,
                screenH,
                clearImageHandle_,
                TRUE
            );
        }
    }
    else
    {
        DrawBox(
            0,
            0,
            screenW,
            screenH,
            GetColor(15, 20, 30),
            TRUE
        );

        const char* errorText =
            "CLEAR IMAGE LOAD ERROR";

        const int textLength =
            static_cast<int>(
                strlen(errorText)
                );

        const int textW =
            GetDrawStringWidth(
                errorText,
                textLength
            );

        DrawString(
            screenW / 2 - textW / 2,
            screenH / 2,
            errorText,
            GetColor(255, 80, 80)
        );
    }

    // 画像を薄く暗くする
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        10
    );

    DrawBox(
        0,
        0,
        screenW,
        screenH,
        GetColor(0, 0, 0),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

void GameClearScene::DrawStaffRoll(void) const
{
    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    const int guideAreaHeight = 76;

    // クレジットを表示する横位置
    const int creditCenterX =
        screenW / 2;

    // 黒い半透明背景は画面全体
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        105
    );

    DrawBox(
        0,
        0,
        screenW,
        screenH,
        GetColor(0, 0, 0),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 下部の案内部分にはスタッフロールを描かない
    SetDrawArea(
        0,
        0,
        screenW,
        screenH - guideAreaHeight
    );

    int y =
        static_cast<int>(
            staffRollY_
            );

    enum class CreditType
    {
        TITLE,
        MESSAGE,
        ROLE,
        NAME,
        SPACE
    };

    struct CreditLine
    {
        CreditType type;
        const char* text;
    };

    const CreditLine credits[] =
    {
        { CreditType::TITLE,   "GAME CLEAR" },
        { CreditType::SPACE,   "" },

        { CreditType::MESSAGE, "プレイしていただき" },
        { CreditType::MESSAGE, "ありがとうございました" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "企画／プロデューサー" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ディレクター" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "キャラクターデザイン" },
        { CreditType::ROLE,    "レッサーパンダデザイン" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "アートディレクター" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プログラムディレクター" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プログラム" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プランニング" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "シナリオ" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "3Dキャラクター" },
        { CreditType::ROLE,    "ディレクター" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "3Dキャラクターモデル" },
        { CreditType::ROLE,    "モーション" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "スペシャルサンクス" },
        { CreditType::NAME,    "遊んでくれた皆様" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::TITLE,   "制作・著作" },
        { CreditType::TITLE,   "しぼあま" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

      
    };

    const int creditCount =
        sizeof(credits) /
        sizeof(credits[0]);

    for (int i = 0; i < creditCount; i++)
    {
        const CreditLine& credit =
            credits[i];

        if (credit.type == CreditType::SPACE)
        {
            y += 30;
            continue;
        }

        int fontHandle =
            nameFontHandle_;

        int textColor =
            GetColor(255, 150, 35);

        int whiteEdgeColor =
            GetColor(255, 245, 210);

        int darkEdgeColor =
            GetColor(80, 35, 5);

        int lineHeight = 66;
        int edgeSize = 3;

        bool hasGreenLine = false;

        switch (credit.type)
        {
        case CreditType::TITLE:
            fontHandle =
                titleFontHandle_;

            textColor =
                GetColor(255, 185, 45);

            whiteEdgeColor =
                GetColor(255, 245, 190);

            darkEdgeColor =
                GetColor(65, 25, 0);

            lineHeight = 115;
            edgeSize = 4;
            hasGreenLine = false;
            break;

        case CreditType::MESSAGE:
            fontHandle =
                roleFontHandle_;

            textColor =
                GetColor(255, 255, 255);

            whiteEdgeColor =
                GetColor(255, 255, 255);

            darkEdgeColor =
                GetColor(20, 20, 20);

            lineHeight = 46;
            edgeSize = 3;
            hasGreenLine = false;
            break;

        case CreditType::ROLE:
            fontHandle =
                roleFontHandle_;

            textColor =
                GetColor(255, 145, 25);

            whiteEdgeColor =
                GetColor(255, 250, 210);

            darkEdgeColor =
                GetColor(90, 35, 0);

            lineHeight = 52;
            edgeSize = 3;
            hasGreenLine = true;
            break;

        case CreditType::NAME:
            fontHandle =
                nameFontHandle_;

            textColor =
                GetColor(255, 135, 20);

            whiteEdgeColor =
                GetColor(255, 245, 205);

            darkEdgeColor =
                GetColor(85, 30, 0);

            lineHeight = 72;
            edgeSize = 4;
            hasGreenLine = true;
            break;

        case CreditType::SPACE:
            break;
        }

        const char* text =
            credit.text;

        const int textLength =
            static_cast<int>(
                strlen(text)
                );

        const int textW =
            GetDrawStringWidthToHandle(
                text,
                textLength,
                fontHandle
            );

        const int drawX =
            creditCenterX - textW / 2;

        if (y > -130 &&
            y < screenH - guideAreaHeight + 40)
        {
            // 黄緑色のラインを文字の後ろに描画
            if (hasGreenLine)
            {
                const int fontSize =
                    GetFontSizeToHandle(
                        fontHandle
                    );

                const int lineY =
                    y + fontSize - 5;

                DrawCreditLine(
                    drawX - 18,
                    lineY,
                    textW + 36,
                    GetColor(130, 255, 70)
                );
            }

            // 外側の暗い縁
            DrawOutlinedText(
                drawX,
                y,
                text,
                darkEdgeColor,
                darkEdgeColor,
                fontHandle,
                edgeSize + 2
            );

            // 内側の白い縁
            DrawOutlinedText(
                drawX,
                y,
                text,
                textColor,
                whiteEdgeColor,
                fontHandle,
                edgeSize
            );
        }

        y += lineHeight;
    }

    // 描画範囲を全画面へ戻す
    SetDrawArea(
        0,
        0,
        screenW,
        screenH
    );
}

void GameClearScene::DrawGuide(void) const
{
    if (!isInputEnabled_)
    {
        return;
    }

    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    const int guideAreaHeight = 76;

    // 下部の黒い帯
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        225
    );

    DrawBox(
        0,
        screenH - guideAreaHeight,
        screenW,
        screenH,
        GetColor(0, 0, 0),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    const float time =
        GetNowCount() / 1000.0f;

    const float blink =
        (sinf(time * 4.0f) + 1.0f)
        * 0.5f;

    const int brightness =
        160 +
        static_cast<int>(
            blink * 95.0f
            );

    const char* guide =
        "決定キー  タイトルへ";

    const int guideLength =
        static_cast<int>(
            strlen(guide)
            );

    const int guideW =
        GetDrawStringWidthToHandle(
            guide,
            guideLength,
            guideFontHandle_
        );

    const int guideX =
        screenW / 2 - guideW / 2;

    const int guideY =
        screenH
        - guideAreaHeight / 2
        - 14;

    DrawOutlinedText(
        guideX,
        guideY,
        guide,
        GetColor(
            brightness,
            brightness,
            brightness
        ),
        GetColor(0, 0, 0),
        guideFontHandle_,
        3
    );
}

void GameClearScene::DrawOutlinedText(
    int x,
    int y,
    const char* text,
    int textColor,
    int edgeColor,
    int fontHandle,
    int edgeSize
) const
{
    /*
     * 縁を隙間なく表示するため、
     * edgeSizeの範囲をすべて描画する。
     */
    for (int offsetY = -edgeSize;
        offsetY <= edgeSize;
        offsetY++)
    {
        for (int offsetX = -edgeSize;
            offsetX <= edgeSize;
            offsetX++)
        {
            if (offsetX == 0 &&
                offsetY == 0)
            {
                continue;
            }

            DrawStringToHandle(
                x + offsetX,
                y + offsetY,
                text,
                edgeColor,
                fontHandle
            );
        }
    }

    // 最後に文字本体を描画
    DrawStringToHandle(
        x,
        y,
        text,
        textColor,
        fontHandle
    );
}

void GameClearScene::DrawCreditLine(
    int x,
    int y,
    int width,
    int color
) const
{
    // 太い半透明ライン
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        75
    );

    DrawLine(
        x,
        y + 2,
        x + width,
        y - 2,
        color,
        14
    );

    // 少しずらしたライン
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        100
    );

    DrawLine(
        x + 8,
        y - 3,
        x + width - 5,
        y + 2,
        color,
        8
    );

    // 中心の明るいライン
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        150
    );

    DrawLine(
        x + 14,
        y,
        x + width - 12,
        y,
        GetColor(190, 255, 100),
        4
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}
