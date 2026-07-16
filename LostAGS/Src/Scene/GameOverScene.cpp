#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "GameOverScene.h"

GameOverScene::GameOverScene(void)
{
    selectIndex_ = 0;
    stickInputWait_ = 0.0f;

    gameOverFontHandle_ = -1;
    menuFontHandle_ = -1;
    guideFontHandle_ = -1;
}
GameOverScene::~GameOverScene(void)
{
    if (gameOverFontHandle_ != -1)
    {
        DeleteFontToHandle(gameOverFontHandle_);
        gameOverFontHandle_ = -1;
    }

    if (menuFontHandle_ != -1)
    {
        DeleteFontToHandle(menuFontHandle_);
        menuFontHandle_ = -1;
    }

    if (guideFontHandle_ != -1)
    {
        DeleteFontToHandle(guideFontHandle_);
        guideFontHandle_ = -1;
    }
}

void GameOverScene::Init(void)
{
    SetMouseDispFlag(TRUE);

    InputManager::GetInstance()
        .SetFixMouse(false);

    selectIndex_ = 0;
    stickInputWait_ = 0.0f;

    if (gameOverFontHandle_ != -1)
    {
        DeleteFontToHandle(
            gameOverFontHandle_
        );
    }

    if (menuFontHandle_ != -1)
    {
        DeleteFontToHandle(
            menuFontHandle_
        );
    }

    if (guideFontHandle_ != -1)
    {
        DeleteFontToHandle(
            guideFontHandle_
        );
    }

    gameOverFontHandle_ =
        CreateFontToHandle(
            "Yu Gothic",
            120,
            9
        );

    menuFontHandle_ =
        CreateFontToHandle(
            "Yu Gothic",
            38,
            6
        );

    guideFontHandle_ =
        CreateFontToHandle(
            "Yu Gothic",
            22,
            3
        );
}


void GameOverScene::Update(void)
{
    InputManager& ins =
        InputManager::GetInstance();

    SceneManager& sceneManager =
        SceneManager::GetInstance();

    stickInputWait_ -=
        sceneManager.GetDeltaTime();

    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    const int centerX =
        screenW / 2;

    const int retryY =
        screenH / 2 + 50;

    const int titleY =
        retryY + 82;

    const int left =
        centerX - 215;

    const int right =
        centerX + 215;

    const int itemHeight = 64;

    /*
     * キーボードとゲームパッドによる選択。
     */
    const int leftStickY =
        ins.GetPadAKeyLY(
            InputManager::JOYPAD_NO::PAD1
        );

    constexpr int deadZone = 8000;

    if (stickInputWait_ <= 0.0f)
    {
        const bool isDown =
            ins.IsTrgDown(KEY_INPUT_DOWN) ||
            leftStickY > deadZone;

        const bool isUp =
            ins.IsTrgDown(KEY_INPUT_UP) ||
            leftStickY < -deadZone;

        if (isDown)
        {
            selectIndex_++;

            if (selectIndex_ > 1)
            {
                selectIndex_ = 0;
            }

            stickInputWait_ = 0.2f;
        }
        else if (isUp)
        {
            selectIndex_--;

            if (selectIndex_ < 0)
            {
                selectIndex_ = 1;
            }

            stickInputWait_ = 0.2f;
        }
    }

    /*
     * マウスカーソルが項目に重なった場合、
     * その項目を選択状態にする。
     */
    int mouseX = 0;
    int mouseY = 0;

    GetMousePoint(
        &mouseX,
        &mouseY
    );

    const bool isRetryHovered =
        mouseX >= left &&
        mouseX <= right &&
        mouseY >= retryY &&
        mouseY <= retryY + itemHeight;

    const bool isTitleHovered =
        mouseX >= left &&
        mouseX <= right &&
        mouseY >= titleY &&
        mouseY <= titleY + itemHeight;

    if (isRetryHovered)
    {
        selectIndex_ = 0;
    }
    else if (isTitleHovered)
    {
        selectIndex_ = 1;
    }

    /*
     * 決定入力。
     */
    const bool isKeyboardOrPadDecide =
        ins.IsTrgDown(KEY_INPUT_RETURN) ||
        ins.IsTrgDown(KEY_INPUT_SPACE) ||
        ins.IsPadBtnTrgDown(
            InputManager::JOYPAD_NO::PAD1,
            InputManager::JOYPAD_BTN::DOWN
        );

    const bool isMouseDecide =
        ins.IsTrgMouseLeft() &&
        (isRetryHovered || isTitleHovered);

    if (!isKeyboardOrPadDecide &&
        !isMouseDecide)
    {
        return;
    }

    if (selectIndex_ == 0)
    {
        sceneManager.ResetGameResultData();

        sceneManager.ChangeScene(
            SceneManager::SCENE_ID::GAME
        );

        return;
    }

    sceneManager.ChangeScene(
        SceneManager::SCENE_ID::TITLE
    );
}

void GameOverScene::Draw(void)
{
    int screenW = 0;
    int screenH = 0;

    GetDrawScreenSize(
        &screenW,
        &screenH
    );

    const int centerX =
        screenW / 2;

    const float time =
        static_cast<float>(
            GetNowCount()
            ) / 1000.0f;

    const int black =
        GetColor(0, 0, 0);

    const int backgroundRed =
        GetColor(45, 0, 0);

    const int darkRed =
        GetColor(100, 0, 0);

    const int red =
        GetColor(220, 15, 20);

    const int brightRed =
        GetColor(255, 55, 60);

    const int white =
        GetColor(225, 225, 225);

    const int gray =
        GetColor(115, 115, 115);

    const int darkGray =
        GetColor(25, 25, 25);

    // 画面全体を黒で塗る
    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    DrawBox(
        0,
        0,
        screenW,
        screenH,
        black,
        TRUE
    );

    // 中央の暗赤色の光
    const float backgroundPulse =
        (sinf(time * 1.4f) + 1.0f)
        * 0.5f;

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        20 +
        static_cast<int>(
            backgroundPulse * 14.0f
            )
    );

    DrawCircle(
        centerX,
        screenH / 2 - 80,
        360,
        backgroundRed,
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        32 +
        static_cast<int>(
            backgroundPulse * 18.0f
            )
    );

    DrawCircle(
        centerX,
        screenH / 2 - 80,
        230,
        darkRed,
        TRUE
    );

    // 画面の上下を暗くする
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        170
    );

    DrawBox(
        0,
        0,
        screenW,
        screenH / 5,
        black,
        TRUE
    );

    DrawBox(
        0,
        screenH - screenH / 5,
        screenW,
        screenH,
        black,
        TRUE
    );

    // 走査線
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        28
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
            black,
            1
        );
    }

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // GAME OVER
    const char* gameOverText =
        "GAME OVER";

    const int gameOverLength =
        static_cast<int>(
            strlen(gameOverText)
            );

    const int gameOverWidth =
        GetDrawStringWidthToHandle(
            gameOverText,
            gameOverLength,
            gameOverFontHandle_
        );

    // ごく小さな揺れ
    const int shakeX =
        static_cast<int>(
            sinf(time * 17.0f) * 1.2f
            );

    const int shakeY =
        static_cast<int>(
            sinf(time * 21.0f) * 0.7f
            );

    const int gameOverX =
        centerX
        - gameOverWidth / 2
        + shakeX;

    const int gameOverY =
        screenH / 2
        - 215
        + shakeY;

    // タイトル背後の影
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        80
    );

    DrawStringToHandle(
        gameOverX + 8,
        gameOverY + 10,
        gameOverText,
        black,
        gameOverFontHandle_
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 黒い縁取り
    constexpr int titleEdgeSize = 6;

    for (int offsetY = -titleEdgeSize;
        offsetY <= titleEdgeSize;
        offsetY++)
    {
        for (int offsetX = -titleEdgeSize;
            offsetX <= titleEdgeSize;
            offsetX++)
        {
            if (offsetX == 0 &&
                offsetY == 0)
            {
                continue;
            }

            if (offsetX * offsetX +
                offsetY * offsetY >
                titleEdgeSize * titleEdgeSize)
            {
                continue;
            }

            DrawStringToHandle(
                gameOverX + offsetX,
                gameOverY + offsetY,
                gameOverText,
                black,
                gameOverFontHandle_
            );
        }
    }

    // 赤い文字を明滅させる
    const float titlePulse =
        (sinf(time * 2.6f) + 1.0f)
        * 0.5f;

    const int titleRed =
        175 +
        static_cast<int>(
            titlePulse * 80.0f
            );

    DrawStringToHandle(
        gameOverX,
        gameOverY,
        gameOverText,
        GetColor(
            titleRed,
            8,
            12
        ),
        gameOverFontHandle_
    );

    // タイトル下の装飾線
    const int dividerY =
        gameOverY + 112;

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        45
    );

    DrawLine(
        centerX - 270,
        dividerY,
        centerX + 270,
        dividerY,
        darkRed,
        8
    );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        160
    );

    DrawLine(
        centerX - 190,
        dividerY,
        centerX + 190,
        dividerY,
        red,
        2
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // メニューパネル位置
    const int retryY =
        screenH / 2 + 55;

    const int titleY =
        retryY + 70;

    const int panelLeft =
        centerX - 170;

    const int panelRight =
        centerX + 170;

    constexpr int panelHeight = 52;

    for (int index = 0;
        index < 2;
        index++)
    {
        const int itemY =
            index == 0
            ? retryY
            : titleY;

        const bool isSelected =
            selectIndex_ == index;

        if (isSelected)
        {
            const float selectionPulse =
                (sinf(time * 4.0f) + 1.0f)
                * 0.5f;

            const int panelAlpha =
                80 +
                static_cast<int>(
                    selectionPulse * 35.0f
                    );

            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                panelAlpha
            );

            DrawBox(
                panelLeft,
                itemY,
                panelRight,
                itemY + panelHeight,
                darkRed,
                TRUE
            );

            // 左端の赤いアクセント
            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                210
            );

            DrawBox(
                panelLeft,
                itemY,
                panelLeft + 5,
                itemY + panelHeight,
                brightRed,
                TRUE
            );

            // 選択枠
            DrawBox(
                panelLeft,
                itemY,
                panelRight,
                itemY + panelHeight,
                red,
                FALSE
            );
        }
        else
        {
            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                125
            );

            DrawBox(
                panelLeft,
                itemY,
                panelRight,
                itemY + panelHeight,
                darkGray,
                TRUE
            );

            SetDrawBlendMode(
                DX_BLENDMODE_ALPHA,
                70
            );

            DrawBox(
                panelLeft,
                itemY,
                panelRight,
                itemY + panelHeight,
                gray,
                FALSE
            );
        }
    }

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // メニュー文字
    const char* retryText =
        "RETRY";

    const char* titleText =
        "TITLE";

    const int retryWidth =
        GetDrawStringWidthToHandle(
            retryText,
            static_cast<int>(
                strlen(retryText)
                ),
            menuFontHandle_
        );

    const int titleWidth =
        GetDrawStringWidthToHandle(
            titleText,
            static_cast<int>(
                strlen(titleText)
                ),
            menuFontHandle_
        );

    const int retryColor =
        selectIndex_ == 0
        ? white
        : gray;

    const int titleColor =
        selectIndex_ == 1
        ? white
        : gray;

    const int menuFontSize =
        GetFontSizeToHandle(
            menuFontHandle_
        );

    const int retryTextY =
        retryY
        + panelHeight / 2
        - menuFontSize / 2;

    const int titleTextY =
        titleY
        + panelHeight / 2
        - menuFontSize / 2;

    DrawStringToHandle(
        centerX - retryWidth / 2,
        retryTextY,
        retryText,
        retryColor,
        menuFontHandle_
    );

    DrawStringToHandle(
        centerX - titleWidth / 2,
        titleTextY,
        titleText,
        titleColor,
        menuFontHandle_
    );

    // 選択マーカー
    const int markerCenterY =
        selectIndex_ == 0
        ? retryY + panelHeight / 2
        : titleY + panelHeight / 2;

    DrawTriangle(
        panelLeft + 22,
        markerCenterY - 8,
        panelLeft + 22,
        markerCenterY + 8,
        panelLeft + 34,
        markerCenterY,
        brightRed,
        TRUE
    );

    // 操作案内
    const char* guide1 =
        "MOVE  :  LEFT STICK / MOUSE";

    const char* guide2 =
        "DECIDE  :  BUTTON / LEFT CLICK";

    const int guide1Width =
        GetDrawStringWidthToHandle(
            guide1,
            static_cast<int>(
                strlen(guide1)
                ),
            guideFontHandle_
        );

    const int guide2Width =
        GetDrawStringWidthToHandle(
            guide2,
            static_cast<int>(
                strlen(guide2)
                ),
            guideFontHandle_
        );

    const int guideY =
        titleY + 100;

    DrawStringToHandle(
        centerX - guide1Width / 2,
        guideY,
        guide1,
        gray,
        guideFontHandle_
    );

    DrawStringToHandle(
        centerX - guide2Width / 2,
        guideY + 30,
        guide2,
        gray,
        guideFontHandle_
    );

    // 描画設定を戻す
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