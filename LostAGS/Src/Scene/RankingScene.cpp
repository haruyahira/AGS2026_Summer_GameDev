#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include <DxLib.h>

#include "RankingScene.h"

#include "../Application.h"
#include "../Manager/InputManager.h"
#include "../Manager/RankingManager.h"
#include "../Manager/SceneManager.h"

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
RankingScene::RankingScene(void)
    :
    timer_(0.0f),
    canInput_(false),

    titleFontHandle_(-1),
    subTitleFontHandle_(-1),
    rankingFontHandle_(-1),
    smallFontHandle_(-1),
    guideFontHandle_(-1)
{
}

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
RankingScene::~RankingScene(void)
{
    if (titleFontHandle_ != -1)
    {
        DeleteFontToHandle(
            titleFontHandle_
        );

        titleFontHandle_ = -1;
    }

    if (subTitleFontHandle_ != -1)
    {
        DeleteFontToHandle(
            subTitleFontHandle_
        );

        subTitleFontHandle_ = -1;
    }

    if (rankingFontHandle_ != -1)
    {
        DeleteFontToHandle(
            rankingFontHandle_
        );

        rankingFontHandle_ = -1;
    }

    if (smallFontHandle_ != -1)
    {
        DeleteFontToHandle(
            smallFontHandle_
        );

        smallFontHandle_ = -1;
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
void RankingScene::Init(void)
{
    timer_ = 0.0f;
    canInput_ = false;

    //---------------------------------
    // フォント
    //---------------------------------
    titleFontHandle_ =
        CreateFontToHandle(
            nullptr,
            62,
            5,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    subTitleFontHandle_ =
        CreateFontToHandle(
            nullptr,
            20,
            2,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    rankingFontHandle_ =
        CreateFontToHandle(
            nullptr,
            34,
            3,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    smallFontHandle_ =
        CreateFontToHandle(
            nullptr,
            17,
            2,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    guideFontHandle_ =
        CreateFontToHandle(
            nullptr,
            24,
            2,
            DX_FONTTYPE_ANTIALIASING_EDGE
        );

    //---------------------------------
    // 保存済みランキングを再読み込み
    //---------------------------------
    RankingManager::GetInstance().
        Reload();
}

//--------------------------------------------------
// ロード完了
//--------------------------------------------------
void RankingScene::OnLoaded(void)
{
}

//--------------------------------------------------
// 更新
//--------------------------------------------------
void RankingScene::Update(void)
{
    const float deltaTime =
        SceneManager::GetInstance().
        GetDeltaTime();

    timer_ +=
        deltaTime;

    //---------------------------------
    // 開いた直後の決定入力を無視
    //---------------------------------
    if (timer_ >= 0.35f)
    {
        canInput_ = true;
    }

    if (!canInput_)
    {
        return;
    }

    InputManager& input =
        InputManager::GetInstance();

    //---------------------------------
    // EnterまたはEscapeで戻る
    //---------------------------------
    if (input.IsTrgDown(
        KEY_INPUT_RETURN) ||
        input.IsTrgDown(
            KEY_INPUT_ESCAPE))
    {
        SceneManager::GetInstance().
            ChangeScene(
                SceneManager::SCENE_ID::TITLE
            );

        return;
    }

#ifdef _DEBUG

    //---------------------------------
    // デバッグ時のみDeleteで全削除
    //---------------------------------
    if (input.IsTrgDown(
        KEY_INPUT_DELETE))
    {
        RankingManager::GetInstance().
            Clear();
    }

#endif
}

//--------------------------------------------------
// 描画
//--------------------------------------------------
void RankingScene::Draw(void)
{
    DrawBackground();
    DrawDecorations();
    DrawPanel();
    DrawHeader();
    DrawRanking();
    DrawGuide();
}

//--------------------------------------------------
// 背景
//--------------------------------------------------
void RankingScene::DrawBackground(void)
{
    const int screenW =
        Application::SCREEN_SIZE_X;

    const int screenH =
        Application::adjustedSizeY_;

    //---------------------------------
    // 深い青緑のグラデーション
    //---------------------------------
    for (int y = 0;
        y < screenH;
        y += 3)
    {
        const float rate =
            static_cast<float>(y) /
            static_cast<float>(screenH);

        const int red =
            static_cast<int>(
                2.0f +
                rate * 5.0f
                );

        const int green =
            static_cast<int>(
                8.0f +
                rate * 15.0f
                );

        const int blue =
            static_cast<int>(
                13.0f +
                rate * 17.0f
                );

        DrawBox(
            0,
            y,
            screenW,
            y + 3,
            GetColor(
                red,
                green,
                blue
            ),
            TRUE
        );
    }

    //---------------------------------
    // 横方向の走査線
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        20
    );

    for (int y = 0;
        y < screenH;
        y += 5)
    {
        DrawLine(
            0,
            y,
            screenW,
            y,
            GetColor(
                55,
                220,
                190
            )
        );
    }

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    //---------------------------------
    // 細い垂直走査光
    //---------------------------------
    const int sweepWidth =
        34;

    const int sweepX =
        static_cast<int>(
            std::fmod(
                timer_ * 110.0f,
                static_cast<float>(
                    screenW +
                    sweepWidth
                    )
            )
            ) -
        sweepWidth;

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        18
    );

    DrawBox(
        sweepX,
        0,
        sweepX + sweepWidth,
        screenH,
        GetColor(
            70,
            255,
            220
        ),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        60
    );

    DrawLine(
        sweepX + sweepWidth / 2,
        0,
        sweepX + sweepWidth / 2,
        screenH,
        GetColor(
            130,
            255,
            235
        )
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    //---------------------------------
    // 上下の暗い帯
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        100
    );

    DrawBox(
        0,
        0,
        screenW,
        78,
        GetColor(
            0,
            0,
            0
        ),
        TRUE
    );

    DrawBox(
        0,
        screenH - 80,
        screenW,
        screenH,
        GetColor(
            0,
            0,
            0
        ),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

//--------------------------------------------------
// 背景装飾
//--------------------------------------------------
void RankingScene::DrawDecorations(void)
{
    const int screenW =
        Application::SCREEN_SIZE_X;

    const int screenH =
        Application::adjustedSizeY_;

    const unsigned int faintColor =
        GetColor(
            20,
            75,
            74
        );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        90
    );

    //---------------------------------
    // 左側の回路風ライン
    //---------------------------------
    DrawLine(
        0,
        145,
        280,
        145,
        faintColor
    );

    DrawLine(
        280,
        145,
        330,
        195,
        faintColor
    );

    DrawLine(
        0,
        screenH - 165,
        250,
        screenH - 165,
        faintColor
    );

    DrawLine(
        250,
        screenH - 165,
        310,
        screenH - 225,
        faintColor
    );

    //---------------------------------
    // 右側の回路風ライン
    //---------------------------------
    DrawLine(
        screenW,
        145,
        screenW - 280,
        145,
        faintColor
    );

    DrawLine(
        screenW - 280,
        145,
        screenW - 330,
        195,
        faintColor
    );

    DrawLine(
        screenW,
        screenH - 165,
        screenW - 250,
        screenH - 165,
        faintColor
    );

    DrawLine(
        screenW - 250,
        screenH - 165,
        screenW - 310,
        screenH - 225,
        faintColor
    );

    //---------------------------------
    // 装飾点
    //---------------------------------
    DrawCircle(
        280,
        145,
        4,
        GetColor(
            60,
            220,
            195
        ),
        TRUE
    );

    DrawCircle(
        screenW - 280,
        145,
        4,
        GetColor(
            60,
            220,
            195
        ),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

//--------------------------------------------------
// パネル
//--------------------------------------------------
void RankingScene::DrawPanel(void)
{
    const int screenW =
        Application::SCREEN_SIZE_X;

    const int screenH =
        Application::adjustedSizeY_;

    const int panelY =
        180;

    const int sideMargin =
        60;

    const int bottomMargin =
        110;

    const int panelW =
        (std::min)(
            920,
            screenW -
            sideMargin * 2
            );

    const int panelH =
        (std::min)(
            770,
            screenH -
            panelY -
            bottomMargin
            );

    const int panelX =
        screenW / 2 -
        panelW / 2;

    //---------------------------------
    // パネル影
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        150
    );

    DrawBox(
        panelX + 14,
        panelY + 16,
        panelX + panelW + 14,
        panelY + panelH + 16,
        GetColor(
            0,
            0,
            0
        ),
        TRUE
    );

    //---------------------------------
    // パネル本体
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        238
    );

    DrawBox(
        panelX,
        panelY,
        panelX + panelW,
        panelY + panelH,
        GetColor(
            3,
            13,
            17
        ),
        TRUE
    );

    //---------------------------------
    // 上部バー
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        180
    );

    DrawBox(
        panelX,
        panelY,
        panelX + panelW,
        panelY + 56,
        GetColor(
            10,
            48,
            51
        ),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    //---------------------------------
    // 外枠
    //---------------------------------
    DrawBox(
        panelX,
        panelY,
        panelX + panelW,
        panelY + panelH,
        GetColor(
            55,
            215,
            185
        ),
        FALSE
    );

    //---------------------------------
    // 内枠
    //---------------------------------
    DrawBox(
        panelX + 5,
        panelY + 5,
        panelX + panelW - 5,
        panelY + panelH - 5,
        GetColor(
            17,
            70,
            74
        ),
        FALSE
    );

    //---------------------------------
    // 上部バー下線
    //---------------------------------
    DrawLine(
        panelX + 24,
        panelY + 56,
        panelX + panelW - 24,
        panelY + 56,
        GetColor(
            60,
            175,
            155
        )
    );

    //---------------------------------
    // RANK列名
    //---------------------------------
    DrawStringToHandle(
        panelX + 48,
        panelY + 18,
        "RANK",
        GetColor(
            105,
            185,
            178
        ),
        smallFontHandle_
    );

    //---------------------------------
    // NAME列名
    //---------------------------------
    DrawStringToHandle(
        panelX + 220,
        panelY + 18,
        "NAME",
        GetColor(
            105,
            185,
            178
        ),
        smallFontHandle_
    );

    //---------------------------------
    // REWARD列名
    //---------------------------------
    const char* rewardLabel =
        "REWARD";

    const int rewardLabelWidth =
        GetDrawStringWidthToHandle(
            rewardLabel,
            static_cast<int>(
                strlen(
                    rewardLabel
                )
                ),
            smallFontHandle_
        );

    DrawStringToHandle(
        panelX +
        panelW -
        rewardLabelWidth -
        48,
        panelY + 18,
        rewardLabel,
        GetColor(
            105,
            185,
            178
        ),
        smallFontHandle_
    );

    //---------------------------------
    // 四隅の装飾
    //---------------------------------
    const int corner =
        46;

    const unsigned int accent =
        GetColor(
            125,
            255,
            225
        );

    //---------------------------------
    // 左上
    //---------------------------------
    DrawLine(
        panelX,
        panelY,
        panelX + corner,
        panelY,
        accent,
        3
    );

    DrawLine(
        panelX,
        panelY,
        panelX,
        panelY + corner,
        accent,
        3
    );

    //---------------------------------
    // 右上
    //---------------------------------
    DrawLine(
        panelX + panelW,
        panelY,
        panelX + panelW - corner,
        panelY,
        accent,
        3
    );

    DrawLine(
        panelX + panelW,
        panelY,
        panelX + panelW,
        panelY + corner,
        accent,
        3
    );

    //---------------------------------
    // 左下
    //---------------------------------
    DrawLine(
        panelX,
        panelY + panelH,
        panelX + corner,
        panelY + panelH,
        accent,
        3
    );

    DrawLine(
        panelX,
        panelY + panelH,
        panelX,
        panelY + panelH - corner,
        accent,
        3
    );

    //---------------------------------
    // 右下
    //---------------------------------
    DrawLine(
        panelX + panelW,
        panelY + panelH,
        panelX + panelW - corner,
        panelY + panelH,
        accent,
        3
    );

    DrawLine(
        panelX + panelW,
        panelY + panelH,
        panelX + panelW,
        panelY + panelH - corner,
        accent,
        3
    );
}

//--------------------------------------------------
// ヘッダー
//--------------------------------------------------
void RankingScene::DrawHeader(void)
{
    const int centerX =
        Application::SCREEN_SIZE_X / 2;

    //---------------------------------
    // 小見出し
    //---------------------------------
    const char* subTitle =
        "SECURITY ARCHIVE / FINANCIAL RECORD";

    const int subTitleWidth =
        GetDrawStringWidthToHandle(
            subTitle,
            static_cast<int>(
                strlen(
                    subTitle
                )
                ),
            subTitleFontHandle_
        );

    DrawStringToHandle(
        centerX -
        subTitleWidth / 2,
        28,
        subTitle,
        GetColor(
            70,
            155,
            148
        ),
        subTitleFontHandle_
    );

    //---------------------------------
    // メインタイトル
    //---------------------------------
    const char* title =
        "MONEY RANKING";

    const int titleWidth =
        GetDrawStringWidthToHandle(
            title,
            static_cast<int>(
                strlen(
                    title
                )
                ),
            titleFontHandle_
        );

    //---------------------------------
    // 影
    //---------------------------------
    DrawStringToHandle(
        centerX -
        titleWidth / 2 +
        4,
        68,
        title,
        GetColor(
            0,
            38,
            34
        ),
        titleFontHandle_
    );

    //---------------------------------
    // 本体
    //---------------------------------
    DrawStringToHandle(
        centerX -
        titleWidth / 2,
        63,
        title,
        GetColor(
            142,
            255,
            225
        ),
        titleFontHandle_
    );

    //---------------------------------
    // タイトル下ライン
    //---------------------------------
    DrawLine(
        centerX - 310,
        146,
        centerX + 310,
        146,
        GetColor(
            28,
            105,
            102
        )
    );

    DrawLine(
        centerX - 85,
        146,
        centerX + 85,
        146,
        GetColor(
            135,
            255,
            225
        ),
        2
    );
}

//--------------------------------------------------
// ランキング表示
//--------------------------------------------------
void RankingScene::DrawRanking(void)
{
    RankingManager& rankingManager =
        RankingManager::GetInstance();

    const int rankingCount =
        rankingManager.GetRankingCount();

    const int centerX =
        Application::SCREEN_SIZE_X / 2;

    const int rowW =
        800;

    const int rowX =
        centerX -
        rowW / 2;

    const int startY =
        250;

    const int rowH =
        54;

    const int rowGap =
        9;

    //---------------------------------
    // 10行固定表示
    //---------------------------------
    for (int index = 0;
        index <
        RankingManager::MAX_RANKING_COUNT;
        ++index)
    {
        const int rank =
            index + 1;

        const bool hasData =
            index <
            rankingCount;

        const int baseY =
            startY +
            index *
            (rowH + rowGap);

        //---------------------------------
        // 行ごとの登場アニメーション
        //---------------------------------
        const float appearDelay =
            0.35f +
            static_cast<float>(
                index
                ) *
            0.07f;

        const float appearRate =
            EaseOutCubic(
                Clamp01(
                    (
                        timer_ -
                        appearDelay
                        ) /
                    0.35f
                )
            );

        const int slideOffsetX =
            static_cast<int>(
                (
                    1.0f -
                    appearRate
                    ) *
                120.0f
                );

        const int x =
            rowX +
            slideOffsetX;

        const int y =
            baseY;

        const int alpha =
            static_cast<int>(
                appearRate *
                255.0f
                );

        if (alpha <= 0)
        {
            continue;
        }

        const unsigned int rankColor =
            GetRankColor(
                rank
            );

        const unsigned int backgroundColor =
            GetRankBackgroundColor(
                rank
            );

        //---------------------------------
        // 行の影
        //---------------------------------
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            static_cast<int>(
                90.0f *
                appearRate
                )
        );

        DrawBox(
            x + 7,
            y + 7,
            x + rowW + 7,
            y + rowH + 7,
            GetColor(
                0,
                0,
                0
            ),
            TRUE
        );

        //---------------------------------
        // 行背景
        //---------------------------------
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            hasData
            ? static_cast<int>(
                190.0f *
                appearRate
                )
            : static_cast<int>(
                72.0f *
                appearRate
                )
        );

        DrawBox(
            x,
            y,
            x + rowW,
            y + rowH,
            backgroundColor,
            TRUE
        );

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0
        );

        //---------------------------------
        // 上位3位の発光
        //---------------------------------
        if (hasData &&
            rank >= 1 &&
            rank <= 3)
        {
            const float glowRate =
                (
                    std::sin(
                        timer_ * 3.5f +
                        static_cast<float>(
                            rank
                            )
                    ) +
                    1.0f
                    ) *
                0.5f;

            unsigned int glowColor = 0;
            unsigned int shineColor = 0;

            //---------------------------------
            // 金、銀、銅
            //---------------------------------
            if (rank == 1)
            {
                glowColor =
                    GetColor(
                        255,
                        215,
                        55
                    );

                shineColor =
                    GetColor(
                        255,
                        245,
                        160
                    );
            }
            else if (rank == 2)
            {
                glowColor =
                    GetColor(
                        220,
                        235,
                        245
                    );

                shineColor =
                    GetColor(
                        250,
                        255,
                        255
                    );
            }
            else
            {
                glowColor =
                    GetColor(
                        220,
                        135,
                        70
                    );

                shineColor =
                    GetColor(
                        255,
                        185,
                        115
                    );
            }

            //---------------------------------
            // 外側の弱い光
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                static_cast<int>(
                    25.0f +
                    glowRate *
                    20.0f
                    )
            );

            DrawBox(
                x - 7,
                y - 7,
                x + rowW + 7,
                y + rowH + 7,
                glowColor,
                FALSE
            );

            DrawBox(
                x - 6,
                y - 6,
                x + rowW + 6,
                y + rowH + 6,
                glowColor,
                FALSE
            );

            //---------------------------------
            // 中間の光
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                static_cast<int>(
                    45.0f +
                    glowRate *
                    30.0f
                    )
            );

            DrawBox(
                x - 3,
                y - 3,
                x + rowW + 3,
                y + rowH + 3,
                glowColor,
                FALSE
            );

            DrawBox(
                x - 2,
                y - 2,
                x + rowW + 2,
                y + rowH + 2,
                glowColor,
                FALSE
            );

            //---------------------------------
            // 強い発光枠
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                static_cast<int>(
                    80.0f +
                    glowRate *
                    60.0f
                    )
            );

            DrawBox(
                x,
                y,
                x + rowW,
                y + rowH,
                glowColor,
                FALSE
            );

            //---------------------------------
            // 内側の薄い発光
            //---------------------------------
            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                static_cast<int>(
                    8.0f +
                    glowRate *
                    10.0f
                    )
            );

            DrawBox(
                x + 2,
                y + 2,
                x + rowW - 2,
                y + rowH - 2,
                glowColor,
                TRUE
            );

            //---------------------------------
            // 流れる光沢
            //---------------------------------
            const int shineWidth =
                90;

            const int shineX =
                x -
                shineWidth +
                static_cast<int>(
                    std::fmod(
                        timer_ *
                        210.0f +
                        static_cast<float>(
                            rank *
                            100
                            ),
                        static_cast<float>(
                            rowW +
                            shineWidth *
                            2
                            )
                    )
                    );

            int shineLeft =
                shineX;

            int shineRight =
                shineX +
                shineWidth;

            if (shineLeft < x + 2)
            {
                shineLeft =
                    x + 2;
            }

            if (shineRight >
                x + rowW - 2)
            {
                shineRight =
                    x + rowW - 2;
            }

            SetDrawBlendMode(
                DX_BLENDMODE_ADD,
                35
            );

            if (shineLeft < shineRight)
            {
                DrawBox(
                    shineLeft,
                    y + 2,
                    shineRight,
                    y + rowH - 2,
                    shineColor,
                    TRUE
                );
            }

            SetDrawBlendMode(
                DX_BLENDMODE_NOBLEND,
                0
            );
        }

        //---------------------------------
        // 行の枠線
        //---------------------------------
        DrawBox(
            x,
            y,
            x + rowW,
            y + rowH,
            hasData
            ? rankColor
            : GetColor(
                22,
                55,
                58
            ),
            FALSE
        );

        //---------------------------------
        // 左アクセント
        //---------------------------------
        DrawBox(
            x,
            y,
            x + 6,
            y + rowH,
            hasData
            ? rankColor
            : GetColor(
                25,
                60,
                63
            ),
            TRUE
        );

        //---------------------------------
        // 順位
        //---------------------------------
        const char* rankName =
            GetRankName(
                rank
            );

        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            alpha
        );

        DrawStringToHandle(
            x + 34,
            y + 7,
            rankName,
            hasData
            ? rankColor
            : GetColor(
                50,
                78,
                79
            ),
            rankingFontHandle_
        );

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0
        );

        //---------------------------------
        // データがある場合
        //---------------------------------
        if (hasData)
        {
            //---------------------------------
            // プレイヤー名
            //---------------------------------
            std::string playerName =
                rankingManager.GetName(
                    rank
                );

            if (playerName.empty())
            {
                playerName =
                    "NO NAME";
            }

            const int nameX =
                x + 150;

            const int nameY =
                y + 7;

            //---------------------------------
            // 上位3位の名前を発光
            //---------------------------------
            if (rank >= 1 &&
                rank <= 3)
            {
                SetDrawBlendMode(
                    DX_BLENDMODE_ADD,
                    55
                );

                DrawStringToHandle(
                    nameX - 1,
                    nameY,
                    playerName.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                DrawStringToHandle(
                    nameX + 1,
                    nameY,
                    playerName.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                DrawStringToHandle(
                    nameX,
                    nameY - 1,
                    playerName.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                DrawStringToHandle(
                    nameX,
                    nameY + 1,
                    playerName.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                SetDrawBlendMode(
                    DX_BLENDMODE_NOBLEND,
                    0
                );
            }

            //---------------------------------
            // 通常の名前
            //---------------------------------
            DrawStringToHandle(
                nameX,
                nameY,
                playerName.c_str(),
                rankColor,
                rankingFontHandle_
            );

            //---------------------------------
            // 金額
            //---------------------------------
            const int money =
                rankingManager.GetMoney(
                    rank
                );

            const std::string moneyText =
                FormatMoney(
                    money
                ) +
                " G";

            const int moneyWidth =
                GetDrawStringWidthToHandle(
                    moneyText.c_str(),
                    static_cast<int>(
                        moneyText.size()
                        ),
                    rankingFontHandle_
                );

            const int moneyX =
                x +
                rowW -
                moneyWidth -
                30;

            const int moneyY =
                y + 7;

            //---------------------------------
            // 上位3位の金額を発光
            //---------------------------------
            if (rank >= 1 &&
                rank <= 3)
            {
                SetDrawBlendMode(
                    DX_BLENDMODE_ADD,
                    55
                );

                DrawStringToHandle(
                    moneyX - 1,
                    moneyY,
                    moneyText.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                DrawStringToHandle(
                    moneyX + 1,
                    moneyY,
                    moneyText.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                DrawStringToHandle(
                    moneyX,
                    moneyY - 1,
                    moneyText.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                DrawStringToHandle(
                    moneyX,
                    moneyY + 1,
                    moneyText.c_str(),
                    rankColor,
                    rankingFontHandle_
                );

                SetDrawBlendMode(
                    DX_BLENDMODE_NOBLEND,
                    0
                );
            }

            //---------------------------------
            // 通常の金額
            //---------------------------------
            DrawStringToHandle(
                moneyX,
                moneyY,
                moneyText.c_str(),
                rankColor,
                rankingFontHandle_
            );
        }
        else
        {
            //---------------------------------
            // 空き順位の区切り線
            //---------------------------------
            DrawLine(
                x + 150,
                y + rowH / 2,
                x + rowW - 240,
                y + rowH / 2,
                GetColor(
                    20,
                    48,
                    50
                )
            );

            //---------------------------------
            // NO DATA
            //---------------------------------
            const char* noData =
                "NO DATA";

            const int noDataWidth =
                GetDrawStringWidthToHandle(
                    noData,
                    static_cast<int>(
                        strlen(
                            noData
                        )
                        ),
                    rankingFontHandle_
                );

            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                alpha
            );

            DrawStringToHandle(
                x +
                rowW -
                noDataWidth -
                30,
                y + 7,
                noData,
                GetColor(
                    46,
                    74,
                    76
                ),
                rankingFontHandle_
            );

            SetDrawBlendMode(
                DX_BLENDMODE_NOBLEND,
                0
            );
        }
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
// 戻る案内
//--------------------------------------------------
void RankingScene::DrawGuide(void)
{
    const float blink =
        (
            std::sin(
                timer_ * 4.0f
            ) +
            1.0f
            ) *
        0.5f;

    const int alpha =
        125 +
        static_cast<int>(
            blink *
            130.0f
            );

    const char* guide =
        "[ ENTER / ESC ]  BACK TO TITLE";

    const int width =
        GetDrawStringWidthToHandle(
            guide,
            static_cast<int>(
                strlen(
                    guide
                )
                ),
            guideFontHandle_
        );

    const int guideX =
        Application::SCREEN_SIZE_X / 2 -
        width / 2;

    const int guideY =
        Application::adjustedSizeY_ -
        72;

    //---------------------------------
    // ガイド背景
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        150
    );

    DrawBox(
        guideX - 30,
        guideY - 11,
        guideX + width + 30,
        guideY + 39,
        GetColor(
            3,
            19,
            22
        ),
        TRUE
    );

    DrawBox(
        guideX - 30,
        guideY - 11,
        guideX + width + 30,
        guideY + 39,
        GetColor(
            28,
            95,
            91
        ),
        FALSE
    );

    //---------------------------------
    // ガイド文字
    //---------------------------------
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        alpha
    );

    DrawStringToHandle(
        guideX,
        guideY,
        guide,
        GetColor(
            185,
            245,
            228
        ),
        guideFontHandle_
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );
}

//--------------------------------------------------
// 順位色
//--------------------------------------------------
unsigned int RankingScene::GetRankColor(
    int rank) const
{
    switch (rank)
    {
    case 1:

        //---------------------------------
        // 金
        //---------------------------------
        return GetColor(
            255,
            215,
            55
        );

    case 2:

        //---------------------------------
        // 銀
        //---------------------------------
        return GetColor(
            220,
            235,
            245
        );

    case 3:

        //---------------------------------
        // 銅
        //---------------------------------
        return GetColor(
            220,
            135,
            70
        );

    default:

        return GetColor(
            135,
            215,
            202
        );
    }
}

//--------------------------------------------------
// 行背景色
//--------------------------------------------------
unsigned int RankingScene::GetRankBackgroundColor(
    int rank) const
{
    switch (rank)
    {
    case 1:

        //---------------------------------
        // 金色系の背景
        //---------------------------------
        return GetColor(
            55,
            47,
            16
        );

    case 2:

        //---------------------------------
        // 銀色系の背景
        //---------------------------------
        return GetColor(
            31,
            41,
            46
        );

    case 3:

        //---------------------------------
        // 銅色系の背景
        //---------------------------------
        return GetColor(
            47,
            29,
            21
        );

    default:

        return GetColor(
            8,
            28,
            32
        );
    }
}

//--------------------------------------------------
// 順位表示
//--------------------------------------------------
const char* RankingScene::GetRankName(
    int rank) const
{
    static const char* rankNames[
        RankingManager::MAX_RANKING_COUNT
    ] =
        {
            "01",
            "02",
            "03",
            "04",
            "05",
            "06",
            "07",
            "08",
            "09",
            "10"
        };

        if (rank < 1 ||
            rank >
            RankingManager::MAX_RANKING_COUNT)
        {
            return "--";
        }

        return rankNames[
            rank - 1
        ];
}

//--------------------------------------------------
// 金額の桁区切り
//--------------------------------------------------
std::string RankingScene::FormatMoney(
    int money) const
{
    if (money < 0)
    {
        money = 0;
    }

    std::string number =
        std::to_string(
            money
        );

    int insertPosition =
        static_cast<int>(
            number.size()
            ) -
        3;

    while (insertPosition > 0)
    {
        number.insert(
            static_cast<std::size_t>(
                insertPosition
                ),
            ","
        );

        insertPosition -= 3;
    }

    return number;
}

//--------------------------------------------------
// 0～1へ制限
//--------------------------------------------------
float RankingScene::Clamp01(
    float value) const
{
    if (value < 0.0f)
    {
        return 0.0f;
    }

    if (value > 1.0f)
    {
        return 1.0f;
    }

    return value;
}

//--------------------------------------------------
// イージング
//--------------------------------------------------
float RankingScene::EaseOutCubic(
    float value) const
{
    const float inverse =
        1.0f -
        value;

    return
        1.0f -
        inverse *
        inverse *
        inverse;
}