#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <DxLib.h>

#include "ResultScene.h"

#include "../Application.h"
#include "../Manager/InputManager.h"
#include "../Manager/RankingManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/SoundManager.h"

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
ResultScene::ResultScene(void)
    :
    step_(STEP::TITLE_IN),

    stepTimer_(0.0f),
    totalTimer_(0.0f),

    finalMoney_(0),
    displayMoney_(0.0f),

    currentRank_(-1),
    visibleRankingCount_(0),

    isRankingRegistered_(false),
    canInput_(false),

    flashAlpha_(0.0f),
    moneyScale_(1.0f),

    titleFontHandle_(-1),
    moneyFontHandle_(-1),
    rankingFontHandle_(-1),
    guideFontHandle_(-1)
{
}

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
ResultScene::~ResultScene(void)
{
    if (titleFontHandle_ != -1)
    {
        DeleteFontToHandle(
            titleFontHandle_
        );

        titleFontHandle_ = -1;
    }

    if (moneyFontHandle_ != -1)
    {
        DeleteFontToHandle(
            moneyFontHandle_
        );

        moneyFontHandle_ = -1;
    }

    if (rankingFontHandle_ != -1)
    {
        DeleteFontToHandle(
            rankingFontHandle_
        );

        rankingFontHandle_ = -1;
    }

    if (guideFontHandle_ != -1)
    {
        DeleteFontToHandle(
            guideFontHandle_
        );

        guideFontHandle_ = -1;
    }
}

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void ResultScene::Init(void)
{
    //---------------------------------
    // 演出状態
    //---------------------------------
    step_ =
        STEP::TITLE_IN;

    stepTimer_ = 0.0f;
    totalTimer_ = 0.0f;

    displayMoney_ = 0.0f;

    currentRank_ = -1;
    visibleRankingCount_ = 0;

    isRankingRegistered_ = false;
    canInput_ = false;

    flashAlpha_ = 0.0f;
    moneyScale_ = 1.0f;

    //---------------------------------
    // フォント作成
    //---------------------------------
    titleFontHandle_ =
        CreateFontToHandle(
            nullptr,
            72,
            5,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    moneyFontHandle_ =
        CreateFontToHandle(
            nullptr,
            64,
            4,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    rankingFontHandle_ =
        CreateFontToHandle(
            nullptr,
            38,
            3,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    guideFontHandle_ =
        CreateFontToHandle(
            nullptr,
            28,
            2,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );
}

//--------------------------------------------------
// ロード完了
//--------------------------------------------------
void ResultScene::OnLoaded(void)
{
    SceneManager& sceneManager =
        SceneManager::GetInstance();

    //---------------------------------
    // 1日で盗んだ最終金額
    //---------------------------------
    finalMoney_ =
        sceneManager.GetResultStolenMoney();

    if (finalMoney_ < 0)
    {
        finalMoney_ = 0;
    }

    displayMoney_ = 0.0f;

    currentRank_ = -1;
    isRankingRegistered_ = false;

    //---------------------------------
    // ゲームクリア時だけランキング登録
    //---------------------------------
    if (sceneManager.IsGameClear())
    {
        RankingManager& rankingManager =
            RankingManager::GetInstance();

        rankingManager.Init();

        currentRank_ =
            rankingManager.RegisterMoney(
                finalMoney_
            );

        rankingManager.Save();

        sceneManager.SetResultRank(
            currentRank_
        );

        isRankingRegistered_ = true;
    }
}

//--------------------------------------------------
// 更新
//--------------------------------------------------
void ResultScene::Update(void)
{
    const float deltaTime =
        SceneManager::GetInstance().
        GetDeltaTime();

    stepTimer_ += deltaTime;
    totalTimer_ += deltaTime;

    //---------------------------------
    // フラッシュを減らす
    //---------------------------------
    flashAlpha_ -=
        500.0f *
        deltaTime;

    if (flashAlpha_ < 0.0f)
    {
        flashAlpha_ = 0.0f;
    }

    //---------------------------------
    // 金額拡大を戻す
    //---------------------------------
    moneyScale_ +=
        (1.0f - moneyScale_) *
        8.0f *
        deltaTime;

    switch (step_)
    {
    case STEP::TITLE_IN:

        UpdateTitleIn();
        break;

    case STEP::MONEY_COUNT:

        UpdateMoneyCount();
        break;

    case STEP::MONEY_FIX:

        UpdateMoneyFix();
        break;

    case STEP::RANKING_SHOW:

        UpdateRankingShow();
        break;

    case STEP::WAIT_INPUT:

        UpdateWaitInput();
        break;

    case STEP::FINISH:

        UpdateFinish();
        break;
    }
}

//--------------------------------------------------
// RESULTタイトル表示
//--------------------------------------------------
void ResultScene::UpdateTitleIn(void)
{
    constexpr float titleWaitTime =
        1.0f;

    if (stepTimer_ >= titleWaitTime)
    {
        step_ =
            STEP::MONEY_COUNT;

        stepTimer_ = 0.0f;
    }
}

//--------------------------------------------------
// 金額カウントアップ
//--------------------------------------------------
void ResultScene::UpdateMoneyCount(void)
{
    const float deltaTime =
        SceneManager::GetInstance().
        GetDeltaTime();

    constexpr float countDuration =
        2.5f;

    const float countSpeed =
        static_cast<float>(finalMoney_) /
        countDuration;

    displayMoney_ +=
        countSpeed *
        deltaTime;

    //---------------------------------
    // 0円のときも停止できるようにする
    //---------------------------------
    if (finalMoney_ <= 0 ||
        displayMoney_ >=
        static_cast<float>(finalMoney_))
    {
        displayMoney_ =
            static_cast<float>(
                finalMoney_
                );

        //---------------------------------
        // 確定演出
        //---------------------------------
        flashAlpha_ = 190.0f;
        moneyScale_ = 1.25f;

        step_ =
            STEP::MONEY_FIX;

        stepTimer_ = 0.0f;

        //---------------------------------
        // 金額確定効果音を入れる場合は
        // ここで再生する
        //---------------------------------
    }
}

//--------------------------------------------------
// 金額確定演出
//--------------------------------------------------
void ResultScene::UpdateMoneyFix(void)
{
    constexpr float fixTime =
        0.8f;

    if (stepTimer_ >= fixTime)
    {
        if (isRankingRegistered_)
        {
            step_ =
                STEP::RANKING_SHOW;

            visibleRankingCount_ = 0;
        }
        else
        {
            step_ =
                STEP::WAIT_INPUT;

            canInput_ = true;
        }

        stepTimer_ = 0.0f;
    }
}

//--------------------------------------------------
// ランキングを順番に表示
//--------------------------------------------------
void ResultScene::UpdateRankingShow(void)
{
    constexpr float oneEntryTime =
        0.30f;

    const int rankingCount =
        RankingManager::GetInstance().
        GetRankingCount();

    if (stepTimer_ >= oneEntryTime)
    {
        stepTimer_ = 0.0f;

        ++visibleRankingCount_;

        if (visibleRankingCount_ >=
            rankingCount)
        {
            visibleRankingCount_ =
                rankingCount;

            step_ =
                STEP::WAIT_INPUT;

            canInput_ = true;
        }
    }
}

//--------------------------------------------------
// 入力待ち
//--------------------------------------------------
void ResultScene::UpdateWaitInput(void)
{
    if (!canInput_)
    {
        return;
    }

    InputManager& input =
        InputManager::GetInstance();

    //---------------------------------
    // 決定入力
    //---------------------------------
    if (input.IsTrgDown(
        KEY_INPUT_RETURN))
    {
        step_ =
            STEP::FINISH;

        stepTimer_ = 0.0f;
    }
}

//--------------------------------------------------
// 次シーンへ
//--------------------------------------------------
void ResultScene::UpdateFinish(void)
{
    SceneManager& sceneManager =
        SceneManager::GetInstance();

    //---------------------------------
    // クリアならスタッフロールへ
    //---------------------------------
    if (sceneManager.IsGameClear())
    {
        sceneManager.ChangeScene(
            SceneManager::SCENE_ID::GAMECLEAR
        );

        return;
    }

    //---------------------------------
    // 条件未達成ならゲームオーバー
    //---------------------------------
    sceneManager.ChangeScene(
        SceneManager::SCENE_ID::GAMEOVER
    );
}

//--------------------------------------------------
// 描画
//--------------------------------------------------
void ResultScene::Draw(void)
{
    DrawBackground();
    DrawTitle();
    DrawMoney();

    if (isRankingRegistered_)
    {
        DrawRanking();
    }

    if (step_ == STEP::WAIT_INPUT)
    {
        DrawNextGuide();
    }

    //---------------------------------
    // 金額確定時のフラッシュ
    //---------------------------------
    if (flashAlpha_ > 0.0f)
    {
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            static_cast<int>(
                flashAlpha_
                )
        );

        DrawBox(
            0,
            0,
            Application::SCREEN_SIZE_X,
            Application::adjustedSizeY_,
            GetColor(255, 240, 160),
            TRUE
        );

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0
        );
    }
}

//--------------------------------------------------
// 背景
//--------------------------------------------------
void ResultScene::DrawBackground(void)
{
    const int screenW =
        Application::SCREEN_SIZE_X;

    const int screenH =
        Application::adjustedSizeY_;

    //---------------------------------
    // 暗いグラデーション
    //---------------------------------
    for (int y = 0; y < screenH; y += 4)
    {
        const float rate =
            static_cast<float>(y) /
            static_cast<float>(screenH);

        const int r =
            static_cast<int>(
                4.0f +
                rate * 8.0f
                );

        const int g =
            static_cast<int>(
                8.0f +
                rate * 20.0f
                );

        const int b =
            static_cast<int>(
                12.0f +
                rate * 20.0f
                );

        DrawBox(
            0,
            y,
            screenW,
            y + 4,
            GetColor(r, g, b),
            TRUE
        );
    }

    //---------------------------------
    // 走査線
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        25
    );

    for (int y = 0;
        y < screenH;
        y += 6)
    {
        DrawLine(
            0,
            y,
            screenW,
            y,
            GetColor(50, 255, 210)
        );
    }

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

//--------------------------------------------------
// RESULTタイトル
//--------------------------------------------------
void ResultScene::DrawTitle(void)
{
    //---------------------------------
    // フェードイン
    //---------------------------------
    float titleRate =
        totalTimer_ / 0.8f;

    if (titleRate > 1.0f)
    {
        titleRate = 1.0f;
    }

    const int alpha =
        static_cast<int>(
            titleRate * 255.0f
            );

    const char* text =
        "MISSION RESULT";

    const int width =
        GetDrawStringWidthToHandle(
            text,
            static_cast<int>(
                strlen(text)
                ),
            titleFontHandle_
        );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        alpha
    );

    DrawStringToHandle(
        Application::SCREEN_SIZE_X / 2 -
        width / 2,
        55,
        text,
        GetColor(130, 255, 220),
        titleFontHandle_
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

//--------------------------------------------------
// 金額表示
//--------------------------------------------------
void ResultScene::DrawMoney(void)
{
    char moneyText[128];

    MakeMoneyText(
        moneyText,
        sizeof(moneyText),
        static_cast<int>(
            displayMoney_
            )
    );

    //---------------------------------
    // 通常サイズでの文字幅
    //---------------------------------
    const int normalWidth =
        GetDrawStringWidthToHandle(
            moneyText,
            static_cast<int>(
                strlen(moneyText)
                ),
            moneyFontHandle_
        );

    //---------------------------------
    // 拡大後の文字幅
    //---------------------------------
    const int extendWidth =
        static_cast<int>(
            static_cast<float>(normalWidth) *
            moneyScale_
            );

    const int centerX =
        Application::SCREEN_SIZE_X / 2;

    const int baseY =
        190;

    //---------------------------------
    // ラベル
    //---------------------------------
    const char* label =
        "TOTAL STOLEN MONEY";

    const int labelWidth =
        GetDrawStringWidthToHandle(
            label,
            static_cast<int>(
                strlen(label)
                ),
            rankingFontHandle_
        );

    DrawStringToHandle(
        centerX - labelWidth / 2,
        145,
        label,
        GetColor(
            150,
            210,
            200
        ),
        rankingFontHandle_
    );

    //---------------------------------
    // 金額を拡大表示
    //
    // x座標は拡大後の文字幅から計算し、
    // 画面中央に配置する
    //---------------------------------
    DrawExtendStringToHandle(
        centerX - extendWidth / 2,
        baseY,
        static_cast<double>(
            moneyScale_
            ),
        static_cast<double>(
            moneyScale_
            ),
        moneyText,
        GetColor(
            255,
            225,
            90
        ),
        moneyFontHandle_
    );
}

//--------------------------------------------------
// ランキング表示
//--------------------------------------------------
void ResultScene::DrawRanking(void)
{
    RankingManager& rankingManager =
        RankingManager::GetInstance();

    const int centerX =
        Application::SCREEN_SIZE_X / 2;

    const int startY =
        330;

    const int lineHeight =
        58;

    //---------------------------------
    // 見出し
    //---------------------------------
    const char* rankingTitle =
        "MONEY RANKING";

    const int titleWidth =
        GetDrawStringWidthToHandle(
            rankingTitle,
            static_cast<int>(
                strlen(rankingTitle)
                ),
            rankingFontHandle_
        );

    DrawStringToHandle(
        centerX - titleWidth / 2,
        startY - 55,
        rankingTitle,
        GetColor(130, 255, 220),
        rankingFontHandle_
    );

    //---------------------------------
    // ランキング
    //---------------------------------
    for (int index = 0;
        index < visibleRankingCount_;
        ++index)
    {
        const int rank =
            index + 1;

        const int money =
            rankingManager.GetMoney(rank);

        char text[128];

        sprintf_s(
            text,
            "%d  %d G",
            rank,
            money
        );

        unsigned int color =
            GetColor(225, 235, 235);

        //---------------------------------
        // 順位カラー
        //---------------------------------
        if (rank == 1)
        {
            color =
                GetColor(255, 215, 70);
        }
        else if (rank == 2)
        {
            color =
                GetColor(210, 225, 235);
        }
        else if (rank == 3)
        {
            color =
                GetColor(210, 135, 75);
        }

        //---------------------------------
        // 今回の順位を強調
        //---------------------------------
        if (rank == currentRank_)
        {
            const float blink =
                (
                    std::sin(
                        totalTimer_ * 6.0f
                    ) +
                    1.0f
                    ) *
                0.5f;

            const int brightness =
                180 +
                static_cast<int>(
                    blink * 75.0f
                    );

            color =
                GetColor(
                    255,
                    brightness,
                    80
                );

            DrawStringToHandle(
                centerX - 330,
                startY +
                index * lineHeight,
                "NEW",
                color,
                rankingFontHandle_
            );
        }

        const int width =
            GetDrawStringWidthToHandle(
                text,
                static_cast<int>(
                    strlen(text)
                    ),
                rankingFontHandle_
            );

        DrawStringToHandle(
            centerX - width / 2,
            startY +
            index * lineHeight,
            text,
            color,
            rankingFontHandle_
        );
    }
}

//--------------------------------------------------
// 次へ進む表示
//--------------------------------------------------
void ResultScene::DrawNextGuide(void)
{
    const float blink =
        (
            std::sin(
                totalTimer_ * 4.0f
            ) +
            1.0f
            ) *
        0.5f;

    const int alpha =
        100 +
        static_cast<int>(
            blink * 155.0f
            );

    const char* guideText = nullptr;

    if (SceneManager::GetInstance().
        IsGameClear())
    {
        guideText =
            "ENTER : STAFF ROLL";
    }
    else
    {
        guideText =
            "ENTER : GAME OVER";
    }

    const int width =
        GetDrawStringWidthToHandle(
            guideText,
            static_cast<int>(
                strlen(guideText)
                ),
            guideFontHandle_
        );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        alpha
    );

    DrawStringToHandle(
        Application::SCREEN_SIZE_X / 2 -
        width / 2,
        Application::adjustedSizeY_ - 80,
        guideText,
        GetColor(
            200,
            255,
            240
        ),
        guideFontHandle_
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

//--------------------------------------------------
// 金額文字列作成
//--------------------------------------------------
void ResultScene::MakeMoneyText(
    char* destination,
    int destinationSize,
    int money) const
{
    if (destination == nullptr ||
        destinationSize <= 0)
    {
        return;
    }

    sprintf_s(
        destination,
        destinationSize,
        "%d G",
        money
    );
}
