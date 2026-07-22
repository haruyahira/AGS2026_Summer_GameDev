#include <DxLib.h>
#include <Windows.h>
#include <cstring>
#include <string>

#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/SoundManager.h"
#include "GameClearScene.h"

namespace
{
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

        { CreditType::ROLE,    "GAME　TITLE" },
        { CreditType::NAME,    "「ぬけだせ！　しぼあま工房」" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "企画" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "キャラクターデザイン/原画"},
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "原案／脚本" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "3Dグラフィック制作" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::NAME,    "白濱 優斗" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プレイヤー／エネミーモデル" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "机" },
        { CreditType::NAME,    "西村 明洋、平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ショーケース" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ロッカー" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "冷凍庫" },
        { CreditType::NAME,    "白濱　優斗" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ゲーミングノートPC" },
        { CreditType::NAME,    "白濱　優斗" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "I６８Book" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "本棚" },
        { CreditType::NAME,    "白濱　優斗" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "緊急脱出装置" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "棚" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ゴミ箱" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "床" },
        { CreditType::NAME,    "白濱 優斗、平崎 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "モーションデザイン" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "進行管理" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },


        { CreditType::ROLE,    "システムグラフィック" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "マップデザイン" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プレイヤー／モブデザイン" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "デバック／テストプレイ" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::NAME,    "白濱 優斗" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プロデューサー" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ディレクター" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プログラム" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "作画監督" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "動画" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "制作進行" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "アニメーションプロデューサー" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "アニメーション制作" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::NAME,    "白濱 優斗" },
        { CreditType::NAME,    "西村 明洋" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "ディレクター" },
        { CreditType::NAME,    "平﨑 晴陽" },
        { CreditType::SPACE,   "" },
        { CreditType::SPACE,   "" },

        { CreditType::ROLE,    "プロデューサー" },
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
 
    constexpr int creditCount =
        static_cast<int>(
            sizeof(credits) / sizeof(credits[0])
            );

    int GetCreditLineHeight(CreditType type)
    {
        switch (type)
        {
        case CreditType::TITLE:
            return 115;

        case CreditType::MESSAGE:
            return 46;

        case CreditType::ROLE:
            return 52;

        case CreditType::NAME:
            return 72;

        case CreditType::SPACE:
            return 30;
        }

        return 0;
    }


}

GameClearScene::GameClearScene(void)
{
    clearImageHandle_ = -1;

    titleFontHandle_ = -1;
    roleFontHandle_ = -1;
    nameFontHandle_ = -1;
    guideFontHandle_ = -1;

    staffRollY_ = 0.0f;
    staffRollSpeed_ = 38.0f;

    finalCreditCenterOffset_ = 0.0f;

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

    const char* fontName = "Yu Gothic";

    // GAME CLEAR、制作・著作など
    titleFontHandle_ = CreateFontToHandle(
        fontName,
        70,
        5
    );

    // 役職、メッセージ
    roleFontHandle_ = CreateFontToHandle(
        fontName,
        32,
        3
    );

    // スタッフ名
    nameFontHandle_ = CreateFontToHandle(
        fontName,
        42,
        3
    );

    // 画面下部の案内
    guideFontHandle_ = CreateFontToHandle(
        fontName,
        30,
        2
    );

    // 最後の表示行の中央位置を自動計算する
    float currentOffsetY = 0.0f;

    finalCreditCenterOffset_ = 0.0f;

    for (int i = 0; i < creditCount; i++)
    {
        const CreditLine& credit =
            credits[i];

        if (credit.type != CreditType::SPACE)
        {
            int fontHandle =
                nameFontHandle_;

            switch (credit.type)
            {
            case CreditType::TITLE:
                fontHandle =
                    titleFontHandle_;
                break;

            case CreditType::MESSAGE:
            case CreditType::ROLE:
                fontHandle =
                    roleFontHandle_;
                break;

            case CreditType::NAME:
                fontHandle =
                    nameFontHandle_;
                break;

            case CreditType::SPACE:
                break;
            }

            const int fontSize =
                GetFontSizeToHandle(
                    fontHandle
                );

            /*
             * 表示行があるたびに更新する。
             * 最終的に最後の表示行の中央位置が残る。
             */
            finalCreditCenterOffset_ =
                currentOffsetY
                + static_cast<float>(fontSize) * 0.5f;
        }

        currentOffsetY +=
            static_cast<float>(
                GetCreditLineHeight(
                    credit.type
                )
                );
    }

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

  

    // スタッフロール停止前だけ上へ動かす
    if (!isStaffRollStopped_)
    {
        staffRollY_ -=
            staffRollSpeed_ * deltaTime;

        // 最後の表示行の現在の中央位置
        const float finalCreditCenterY =
            staffRollY_
            + finalCreditCenterOffset_;

        // 画面全体の縦中央
        const float screenCenterY =
            static_cast<float>(screenH) * 0.5f;

        // 最後の表示行が画面中央に到達したら停止
        if (finalCreditCenterY <= screenCenterY)
        {
            staffRollY_ =
                screenCenterY
                - finalCreditCenterOffset_;

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
        ins.IsClickMouseLeft() ||
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

    // 文字を揺らすための経過時間
    const float waveTime =
        static_cast<float>(GetNowCount()) / 1000.0f;

    for (int i = 0; i < creditCount; i++)
    {
        const CreditLine& credit =
            credits[i];

        if (credit.type == CreditType::SPACE)
        {
            y +=
                GetCreditLineHeight(
                    credit.type
                );

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

        const int lineHeight =
            GetCreditLineHeight(
                credit.type
            );
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

      
        // 文字単位の揺れ幅を考慮して画面内か判定する
        constexpr int waveMarginY = 12;

        if (y > -130 - waveMarginY &&
            y < screenH - guideAreaHeight + 40 + waveMarginY)
        {
            /*
             * 黄緑色のラインは揺らさず、
             * 文字列全体の中央に表示する。
             */
            if (hasGreenLine)
            {
                const int fontSize =
                    GetFontSizeToHandle(
                        fontHandle
                    );

                const int lineX =
                    creditCenterX
                    - textW / 2
                    - 18;

                const int lineY =
                    y + fontSize + 3;

                DrawCreditLine(
                    lineX,
                    lineY,
                    textW + 36,
                    GetColor(130, 255, 70)
                );
            }

            // 1文字ずつ外側の暗い縁を描画
            DrawWavingOutlinedText(
                creditCenterX,
                y,
                text,
                darkEdgeColor,
                darkEdgeColor,
                fontHandle,
                edgeSize + 2,
                waveTime,
                i
            );

            // 1文字ずつ内側の白い縁と文字本体を描画
            DrawWavingOutlinedText(
                creditCenterX,
                y,
                text,
                textColor,
                whiteEdgeColor,
                fontHandle,
                edgeSize,
                waveTime,
                i
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
        "左クリック／A タイトルへ";

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

void GameClearScene::DrawWavingOutlinedText(
    int centerX,
    int baseY,
    const char* text,
    int textColor,
    int edgeColor,
    int fontHandle,
    int edgeSize,
    float waveTime,
    int lineIndex
) const
{
    if (text == nullptr ||
        text[0] == '\0')
    {
        return;
    }

    const int textLength =
        static_cast<int>(
            strlen(text)
            );

    const int totalWidth =
        GetDrawStringWidthToHandle(
            text,
            textLength,
            fontHandle
        );

    int currentX =
        centerX - totalWidth / 2;

    int byteIndex = 0;
    int characterIndex = 0;

    while (byteIndex < textLength)
    {
        const unsigned char firstByte =
            static_cast<unsigned char>(
                text[byteIndex]
                );

        int characterBytes = 1;

        /*
         * Shift-JISの先頭バイトなら2バイト文字。
         * 現在のプロジェクトでは、日本語が通常描画できるため、
         * 実際の文字列がShift-JISになっている可能性が高い。
         */
        if ((firstByte >= 0x81 &&
            firstByte <= 0x9F) ||
            (firstByte >= 0xE0 &&
                firstByte <= 0xFC))
        {
            characterBytes = 2;
        }

        // 文字列の終端を超えないようにする
        if (byteIndex + characterBytes >
            textLength)
        {
            characterBytes = 1;
        }

        char characterText[3] =
        {
            '\0',
            '\0',
            '\0'
        };

        characterText[0] =
            text[byteIndex];

        if (characterBytes == 2)
        {
            characterText[1] =
                text[byteIndex + 1];
        }

        const int characterWidth =
            GetDrawStringWidthToHandle(
                characterText,
                characterBytes,
                fontHandle
            );

        const float phase =
            static_cast<float>(lineIndex) * 0.25f
            + static_cast<float>(characterIndex) * 0.55f;

        const float waveX =
            sinf(
                waveTime * 1.3f
                + phase * 0.7f
            ) * 1.5f;

        const float waveY =
            sinf(
                waveTime * 2.0f
                + phase
            ) * 6.0f;

        const int drawX =
            currentX
            + static_cast<int>(waveX);

        const int drawY =
            baseY
            + static_cast<int>(waveY);

        DrawOutlinedText(
            drawX,
            drawY,
            characterText,
            textColor,
            edgeColor,
            fontHandle,
            edgeSize
        );

        currentX += characterWidth;
        byteIndex += characterBytes;
        characterIndex++;
    }
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
    if (text == nullptr ||
        text[0] == '\0')
    {
        return;
    }

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

    DrawStringToHandle(
        x,
        y,
        text,
        textColor,
        fontHandle
    );
}