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
    guideFontHandle_(-1),

    nameInputHandle_(-1),
    playerName_(""),
    isNameRegistered_(false),
    nameFontHandle_(-1)
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

    //---------------------------------
// 名前入力ハンドル削除
//---------------------------------
    if (nameInputHandle_ != -1)
    {
        DeleteKeyInput(
            nameInputHandle_
        );

        nameInputHandle_ = -1;
    }

    //---------------------------------
    // 名前入力用フォント削除
    //---------------------------------
    if (nameFontHandle_ != -1)
    {
        DeleteFontToHandle(
            nameFontHandle_
        );

        nameFontHandle_ = -1;
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
// 名前入力状態
//---------------------------------
    if (nameInputHandle_ != -1)
    {
        DeleteKeyInput(
            nameInputHandle_
        );

        nameInputHandle_ = -1;
    }

    playerName_.clear();
    isNameRegistered_ = false;

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

    nameFontHandle_ =
        CreateFontToHandle(
            nullptr,
            56,
            3,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

}

//--------------------------------------------------
// ロード完了
//--------------------------------------------------
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
    visibleRankingCount_ = 0;

    isRankingRegistered_ = false;
    isNameRegistered_ = false;

    playerName_.clear();

    //---------------------------------
    // 既存ランキングを読み込む
    //---------------------------------
    RankingManager::GetInstance().
        Init();
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

    case STEP::NAME_INPUT:

        UpdateNameInput();
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
//--------------------------------------------------
// 金額確定演出
//--------------------------------------------------
void ResultScene::UpdateMoneyFix(void)
{
    constexpr float fixTime =
        0.8f;

    if (stepTimer_ < fixTime)
    {
        return;
    }

    SceneManager& sceneManager =
        SceneManager::GetInstance();

    //---------------------------------
    // ゲームクリア時は名前入力へ
    //---------------------------------
    if (sceneManager.IsGameClear())
    {
        //---------------------------------
        // 名前入力領域を作成
        //---------------------------------
        nameInputHandle_ =
            MakeKeyInput(
                12,
                FALSE,
                FALSE,
                FALSE
            );

        //---------------------------------
        // 名前入力を有効にする
        //---------------------------------
        if (nameInputHandle_ != -1)
        {
            SetActiveKeyInput(
                nameInputHandle_
            );

            step_ =
                STEP::NAME_INPUT;
        }
        else
        {
            //---------------------------------
            // 作成失敗時はNO NAMEで登録
            //---------------------------------
            playerName_ =
                "NO NAME";

            RankingManager& rankingManager =
                RankingManager::GetInstance();

            currentRank_ =
                rankingManager.RegisterRecord(
                    playerName_,
                    finalMoney_
                );

            rankingManager.Save();

            sceneManager.SetResultRank(
                currentRank_
            );

            isRankingRegistered_ = true;
            isNameRegistered_ = true;

            visibleRankingCount_ = 0;

            step_ =
                STEP::RANKING_SHOW;
        }
    }
    else
    {
        //---------------------------------
        // ゲームオーバー時は名前入力なし
        //---------------------------------
        step_ =
            STEP::WAIT_INPUT;

        canInput_ = true;
    }

    stepTimer_ = 0.0f;
}

//--------------------------------------------------
// 名前入力
//--------------------------------------------------
void ResultScene::UpdateNameInput(void)
{
    //---------------------------------
    // 入力ハンドルがない場合
    //---------------------------------
    if (nameInputHandle_ == -1)
    {
        return;
    }

    //---------------------------------
    // 入力状態を確認
    //
    // 0: 入力中
    // 1: 入力完了
    // 2: キャンセル
    //---------------------------------
    const int inputState =
        CheckKeyInput(
            nameInputHandle_
        );

    //---------------------------------
    // まだ入力中
    //---------------------------------
    if (inputState == 0)
    {
        return;
    }

    //---------------------------------
    // 入力された名前を取得
    //---------------------------------
    char inputName[256] = {};

    if (inputState == 1)
    {
        GetKeyInputString(
            inputName,
            nameInputHandle_
        );

        playerName_ =
            inputName;
    }

    //---------------------------------
    // 空文字またはキャンセル時
    //---------------------------------
    if (playerName_.empty())
    {
        playerName_ =
            "NO NAME";
    }

    //---------------------------------
    // 名前入力を終了
    //---------------------------------
    SetActiveKeyInput(
        -1
    );

    DeleteKeyInput(
        nameInputHandle_
    );

    nameInputHandle_ = -1;

    //---------------------------------
    // ランキング登録
    //---------------------------------
    RankingManager& rankingManager =
        RankingManager::GetInstance();

    currentRank_ =
        rankingManager.RegisterRecord(
            playerName_,
            finalMoney_
        );

    rankingManager.Save();

    //---------------------------------
    // 今回の順位を保存
    //---------------------------------
    SceneManager::GetInstance().
        SetResultRank(
            currentRank_
        );

    isRankingRegistered_ = true;
    isNameRegistered_ = true;

    //---------------------------------
    // ランキング表示へ進む
    //---------------------------------
    visibleRankingCount_ = 0;

    step_ =
        STEP::RANKING_SHOW;

    stepTimer_ = 0.0f;
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
    //---------------------------------
    // 名前入力画面
    //---------------------------------
    if (step_ == STEP::NAME_INPUT)
    {
        DrawNameInput();
    }

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
    // 各列の固定位置
    //---------------------------------
    const int newBadgeRight =
        centerX - 330;

    const int rankX =
        centerX - 260;

    const int nameX =
        centerX - 190;

    const int moneyRightX =
        centerX + 330;

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

        const int drawY =
            startY +
            index * lineHeight;

        std::string name =
            rankingManager.GetName(
                rank
            );

        if (name.empty())
        {
            name =
                "NO NAME";
        }

        const int money =
            rankingManager.GetMoney(
                rank
            );

        //---------------------------------
        // 順位文字列
        //---------------------------------
        char rankText[16];

        sprintf_s(
            rankText,
            sizeof(rankText),
            "%d",
            rank
        );

        //---------------------------------
        // 金額文字列
        //---------------------------------
        char moneyText[64];

        sprintf_s(
            moneyText,
            sizeof(moneyText),
            "%d G",
            money
        );

        //---------------------------------
        // 順位カラー
        //---------------------------------
        unsigned int color =
            GetColor(
                225,
                235,
                235
            );

        if (rank == 1)
        {
            color =
                GetColor(
                    255,
                    215,
                    70
                );
        }
        else if (rank == 2)
        {
            color =
                GetColor(
                    210,
                    225,
                    235
                );
        }
        else if (rank == 3)
        {
            color =
                GetColor(
                    210,
                    135,
                    75
                );
        }

        //---------------------------------
        // 今回の順位
        //---------------------------------
        const bool isNewRecord =
            rank == currentRank_;

        if (isNewRecord)
        {
            const float blink =
                (
                    std::sin(
                        totalTimer_ * 7.0f
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
                    65
                );

            //---------------------------------
            // NEWバッジの位置
            //---------------------------------
            const int badgeWidth =
                105;

            const int badgeHeight =
                42;

            const int badgeLeft =
                newBadgeRight -
                badgeWidth;

            const int badgeTop =
                drawY + 2;

            const int badgeBottom =
                badgeTop +
                badgeHeight;

            //---------------------------------
            // NEWバッジの外側発光
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                45 +
                static_cast<int>(
                    blink * 55.0f
                    )
            );

            DrawBox(
                badgeLeft - 5,
                badgeTop - 5,
                newBadgeRight + 5,
                badgeBottom + 5,
                color,
                FALSE
            );

            DrawBox(
                badgeLeft - 3,
                badgeTop - 3,
                newBadgeRight + 3,
                badgeBottom + 3,
                color,
                FALSE
            );

            SetDrawBlendMode(
                DX_BLENDMODE_NOBLEND,
                0
            );

            //---------------------------------
            // NEWバッジ背景
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                170
            );

            DrawBox(
                badgeLeft,
                badgeTop,
                newBadgeRight,
                badgeBottom,
                GetColor(
                    75,
                    48,
                    4
                ),
                TRUE
            );

            SetDrawBlendMode(
                DX_BLENDMODE_NOBLEND,
                0
            );

            //---------------------------------
            // NEWバッジ枠
            //---------------------------------
            DrawBox(
                badgeLeft,
                badgeTop,
                newBadgeRight,
                badgeBottom,
                color,
                FALSE
            );

            //---------------------------------
            // NEW文字
            //---------------------------------
            const char* newText =
                "NEW";

            const int newTextWidth =
                GetDrawStringWidthToHandle(
                    newText,
                    static_cast<int>(
                        strlen(newText)
                        ),
                    guideFontHandle_
                );

            const int newTextX =
                badgeLeft +
                (
                    badgeWidth -
                    newTextWidth
                    ) /
                2;

            const int newTextY =
                badgeTop + 6;

            //---------------------------------
            // NEW文字の発光
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                80
            );

            DrawStringToHandle(
                newTextX - 1,
                newTextY,
                newText,
                color,
                guideFontHandle_
            );

            DrawStringToHandle(
                newTextX + 1,
                newTextY,
                newText,
                color,
                guideFontHandle_
            );

            DrawStringToHandle(
                newTextX,
                newTextY - 1,
                newText,
                color,
                guideFontHandle_
            );

            DrawStringToHandle(
                newTextX,
                newTextY + 1,
                newText,
                color,
                guideFontHandle_
            );

            SetDrawBlendMode(
                DX_BLENDMODE_NOBLEND,
                0
            );

            //---------------------------------
            // NEW文字本体
            //---------------------------------
            DrawStringToHandle(
                newTextX,
                newTextY,
                newText,
                GetColor(
                    255,
                    250,
                    190
                ),
                guideFontHandle_
            );

            //---------------------------------
            // 今回の行の背景
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                12 +
                static_cast<int>(
                    blink * 10.0f
                    )
            );

            DrawBox(
                rankX - 20,
                drawY,
                moneyRightX + 15,
                drawY + 46,
                color,
                TRUE
            );

            SetDrawBlendMode(
                DX_BLENDMODE_NOBLEND,
                0
            );
        }

        //---------------------------------
        // 順位を描画
        //---------------------------------
        DrawStringToHandle(
            rankX,
            drawY,
            rankText,
            color,
            rankingFontHandle_
        );

        //---------------------------------
        // 名前を描画
        //---------------------------------
        DrawStringToHandle(
            nameX,
            drawY,
            name.c_str(),
            color,
            rankingFontHandle_
        );

        //---------------------------------
        // 金額を右揃え
        //---------------------------------
        const int moneyWidth =
            GetDrawStringWidthToHandle(
                moneyText,
                static_cast<int>(
                    strlen(moneyText)
                    ),
                rankingFontHandle_
            );

        DrawStringToHandle(
            moneyRightX -
            moneyWidth,
            drawY,
            moneyText,
            color,
            rankingFontHandle_
        );
    }

    //---------------------------------
    // 描画モードを元に戻す
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
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

//--------------------------------------------------
// 名前入力描画
//--------------------------------------------------
void ResultScene::DrawNameInput(void)
{
    if (nameInputHandle_ == -1)
    {
        return;
    }

    const int centerX =
        Application::SCREEN_SIZE_X / 2;

    //---------------------------------
    // 名前入力案内
    //---------------------------------
    const char* titleText =
        "ENTER YOUR NAME";

    const int titleWidth =
        GetDrawStringWidthToHandle(
            titleText,
            static_cast<int>(
                strlen(titleText)
                ),
            nameFontHandle_
        );

    //---------------------------------
    // タイトルの弱い発光
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        50
    );

    DrawStringToHandle(
        centerX - titleWidth / 2 - 1,
        337,
        titleText,
        GetColor(90, 255, 220),
        nameFontHandle_
    );

    DrawStringToHandle(
        centerX - titleWidth / 2 + 1,
        337,
        titleText,
        GetColor(90, 255, 220),
        nameFontHandle_
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    //---------------------------------
    // タイトル本体
    //---------------------------------
    DrawStringToHandle(
        centerX - titleWidth / 2,
        337,
        titleText,
        GetColor(130, 255, 220),
        nameFontHandle_
    );

    //---------------------------------
    // 入力欄の座標
    //---------------------------------
    const int inputLeft =
        centerX - 360;

    const int inputTop =
        420;

    const int inputRight =
        centerX + 360;

    const int inputBottom =
        510;

    //---------------------------------
    // 入力欄の影
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        130
    );

    DrawBox(
        inputLeft + 8,
        inputTop + 8,
        inputRight + 8,
        inputBottom + 8,
        GetColor(0, 0, 0),
        TRUE
    );

    //---------------------------------
    // 入力欄の背景
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        235
    );

    DrawBox(
        inputLeft,
        inputTop,
        inputRight,
        inputBottom,
        GetColor(14, 42, 43),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    //---------------------------------
    // 入力欄の外側の発光
    //---------------------------------
    const float borderBlink =
        (
            std::sin(
                totalTimer_ * 3.5f
            ) +
            1.0f
            ) *
        0.5f;

    SetDrawBlendMode(
        DX_BLENDMODE_ADD,
        35 +
        static_cast<int>(
            borderBlink * 30.0f
            )
    );

    DrawBox(
        inputLeft - 3,
        inputTop - 3,
        inputRight + 3,
        inputBottom + 3,
        GetColor(70, 255, 220),
        FALSE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    //---------------------------------
    // 入力欄の枠
    //---------------------------------
    DrawBox(
        inputLeft,
        inputTop,
        inputRight,
        inputBottom,
        GetColor(130, 255, 220),
        FALSE
    );

    //---------------------------------
    // 入力中の文字列を取得
    //---------------------------------
    char inputText[256] = {};

    GetKeyInputString(
        inputText,
        nameInputHandle_
    );

    //---------------------------------
    // 入力文字の座標
    //---------------------------------
    const int textX =
        inputLeft + 25;

    const int textY =
        inputTop + 14;

    //---------------------------------
    // 入力文字を大きなフォントで描画
    //---------------------------------
    DrawStringToHandle(
        textX,
        textY,
        inputText,
        GetColor(235, 255, 248),
        nameFontHandle_
    );

    //---------------------------------
    // 入力カーソルを点滅表示
    //---------------------------------
    const bool showCursor =
        static_cast<int>(
            totalTimer_ * 2.0f
            ) %
        2 == 0;

    if (showCursor)
    {
        const int inputTextWidth =
            GetDrawStringWidthToHandle(
                inputText,
                static_cast<int>(
                    strlen(inputText)
                    ),
                nameFontHandle_
            );

        DrawBox(
            textX +
            inputTextWidth +
            5,
            textY + 4,
            textX +
            inputTextWidth +
            9,
            textY + 58,
            GetColor(160, 255, 230),
            TRUE
        );
    }

    //---------------------------------
    // 操作案内
    //---------------------------------
    const char* guideText =
        "ENTER : DECIDE";

    const int guideWidth =
        GetDrawStringWidthToHandle(
            guideText,
            static_cast<int>(
                strlen(guideText)
                ),
            guideFontHandle_
        );

    DrawStringToHandle(
        centerX - guideWidth / 2,
        545,
        guideText,
        GetColor(200, 225, 218),
        guideFontHandle_
    );
}