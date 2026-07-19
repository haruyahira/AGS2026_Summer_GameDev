#include "PauseMenu.h"
#include "../Application.h"
#include "../Manager/SoundManager.h"

#include <DxLib.h>
#include <algorithm>
#include <cmath>

namespace
{
    // ========================================
    // メインメニュー
    // ========================================

    constexpr int MAIN_PANEL_WIDTH = 720;
    constexpr int MAIN_PANEL_HEIGHT = 520;

    constexpr int MAIN_BUTTON_WIDTH = 460;
    constexpr int MAIN_BUTTON_HEIGHT = 72;
    constexpr int MAIN_BUTTON_SPACE = 24;

    // ========================================
    // 設定画面
    // ========================================

    constexpr int SETTING_PANEL_WIDTH = 820;
    constexpr int SETTING_PANEL_HEIGHT = 620;

    // ========================================
    // 終了確認
    // ========================================

    constexpr int CONFIRM_PANEL_WIDTH = 720;
    constexpr int CONFIRM_PANEL_HEIGHT = 400;

    // ========================================
    // 音量倍率
    // ========================================

    constexpr int MIN_SCALE_STEP = 0;
    constexpr int MAX_SCALE_STEP = 20;
    constexpr int NORMAL_SCALE_STEP = 10;
}

PauseMenu::PauseMenu()
{
    isActive_ = false;
    menuState_ = MenuState::Main;

    previousEsc_ = false;
    previousMouseLeft_ = false;

    isDraggingBGM_ = false;
    isDraggingSE_ = false;

    // 10が1.0倍
    bgmScaleStep_ = NORMAL_SCALE_STEP;
    seScaleStep_ = NORMAL_SCALE_STEP;
}

void PauseMenu::Init()
{
    isActive_ = false;
    menuState_ = MenuState::Main;

    previousEsc_ = false;
    previousMouseLeft_ = false;

    isDraggingBGM_ = false;
    isDraggingSE_ = false;

    SoundManager& soundManager =
        SoundManager::GetInstance();

    // 現在SoundManagerが持っている倍率を取得
    bgmScaleStep_ =
        static_cast<int>(
            soundManager.GetBGMVolumeScale() *
            10.0f +
            0.5f
            );

    seScaleStep_ =
        static_cast<int>(
            soundManager.GetSEVolumeScale() *
            10.0f +
            0.5f
            );

    bgmScaleStep_ =
        std::clamp(
            bgmScaleStep_,
            MIN_SCALE_STEP,
            MAX_SCALE_STEP
        );

    seScaleStep_ =
        std::clamp(
            seScaleStep_,
            MIN_SCALE_STEP,
            MAX_SCALE_STEP
        );
}

PauseMenu::Result PauseMenu::Update()
{
    const bool currentEsc =
        CheckHitKey(KEY_INPUT_ESCAPE) != 0;

    const bool escPressed =
        currentEsc &&
        !previousEsc_;

    int mouseX = 0;
    int mouseY = 0;

    GetMousePoint(
        &mouseX,
        &mouseY
    );

    const bool currentMouseLeft =
        (GetMouseInput() &
            MOUSE_INPUT_LEFT) != 0;

    const bool mousePressed =
        currentMouseLeft &&
        !previousMouseLeft_;

    Result result =
        Result::None;

    // ========================================
    // ESCキー
    // ========================================

    if (escPressed)
    {
        if (!isActive_)
        {
            Open();
        }
        else if (menuState_ == MenuState::Main)
        {
            Close();
            result = Result::Resume;
        }
        else if (menuState_ == MenuState::Settings)
        {
            menuState_ =
                MenuState::Main;

            isDraggingBGM_ = false;
            isDraggingSE_ = false;
        }
        else if (menuState_ == MenuState::QuitConfirm)
        {
            // 終了確認をキャンセル
            menuState_ =
                MenuState::Main;
        }
    }

    // ========================================
    // 各画面の更新
    // ========================================

    if (isActive_)
    {
        if (menuState_ == MenuState::Main)
        {
            const Result menuResult =
                UpdateMainMenu(
                    mouseX,
                    mouseY,
                    mousePressed
                );

            if (menuResult != Result::None)
            {
                result = menuResult;
            }
        }
        else if (menuState_ == MenuState::Settings)
        {
            UpdateSettings(
                mouseX,
                mouseY,
                currentMouseLeft,
                mousePressed
            );
        }
        else if (menuState_ == MenuState::QuitConfirm)
        {
            const Result quitResult =
                UpdateQuitConfirm(
                    mouseX,
                    mouseY,
                    mousePressed
                );

            if (quitResult != Result::None)
            {
                result = quitResult;
            }
        }
    }

    previousEsc_ =
        currentEsc;

    previousMouseLeft_ =
        currentMouseLeft;

    return result;
}

PauseMenu::Result PauseMenu::UpdateMainMenu(
    int mouseX,
    int mouseY,
    bool mousePressed)
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    const int panelTop =
        screenHeight / 2 -
        MAIN_PANEL_HEIGHT / 2;

    const int buttonLeft =
        screenWidth / 2 -
        MAIN_BUTTON_WIDTH / 2;

    const int buttonRight =
        buttonLeft +
        MAIN_BUTTON_WIDTH;

    const int continueTop =
        panelTop + 150;

    const int settingTop =
        continueTop +
        MAIN_BUTTON_HEIGHT +
        MAIN_BUTTON_SPACE;

    const int quitTop =
        settingTop +
        MAIN_BUTTON_HEIGHT +
        MAIN_BUTTON_SPACE;

    // ========================================
    // ゲームを続ける
    // ========================================

    if (IsButtonClicked(
        mouseX,
        mouseY,
        mousePressed,
        buttonLeft,
        continueTop,
        buttonRight,
        continueTop + MAIN_BUTTON_HEIGHT))
    {
        Close();

        return Result::Resume;
    }

    // ========================================
    // 設定
    // ========================================

    if (IsButtonClicked(
        mouseX,
        mouseY,
        mousePressed,
        buttonLeft,
        settingTop,
        buttonRight,
        settingTop + MAIN_BUTTON_HEIGHT))
    {
        menuState_ =
            MenuState::Settings;

        isDraggingBGM_ = false;
        isDraggingSE_ = false;

        return Result::None;
    }

    // ========================================
    // ゲームをやめる
    // ========================================

    if (IsButtonClicked(
        mouseX,
        mouseY,
        mousePressed,
        buttonLeft,
        quitTop,
        buttonRight,
        quitTop + MAIN_BUTTON_HEIGHT))
    {
        // すぐには終了せず、確認画面を表示
        menuState_ =
            MenuState::QuitConfirm;

        return Result::None;
    }

    return Result::None;
}

void PauseMenu::UpdateSettings(
    int mouseX,
    int mouseY,
    bool mouseDown,
    bool mousePressed)
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    const int panelTop =
        screenHeight / 2 -
        SETTING_PANEL_HEIGHT / 2;

    const int sliderWidth = 560;

    const int sliderX =
        screenWidth / 2 -
        sliderWidth / 2;

    const int bgmSliderY =
        panelTop + 200;

    const int seSliderY =
        panelTop + 355;

    // ========================================
    // BGM倍率
    // ========================================

    UpdateVolumeSlider(
        mouseX,
        mouseY,
        mouseDown,
        isDraggingBGM_,
        sliderX,
        bgmSliderY,
        sliderWidth,
        bgmScaleStep_
    );

    // ========================================
    // SE倍率
    // ========================================

    UpdateVolumeSlider(
        mouseX,
        mouseY,
        mouseDown,
        isDraggingSE_,
        sliderX,
        seSliderY,
        sliderWidth,
        seScaleStep_
    );

    // 倍率をサウンドへ反映
    UpdateSoundVolume();

    // ========================================
    // 戻るボタン
    // ========================================

    const int buttonWidth = 340;
    const int buttonHeight = 70;

    const int buttonLeft =
        screenWidth / 2 -
        buttonWidth / 2;

    const int buttonRight =
        buttonLeft +
        buttonWidth;

    const int buttonTop =
        panelTop + 485;

    if (IsButtonClicked(
        mouseX,
        mouseY,
        mousePressed,
        buttonLeft,
        buttonTop,
        buttonRight,
        buttonTop + buttonHeight))
    {
        menuState_ =
            MenuState::Main;

        isDraggingBGM_ = false;
        isDraggingSE_ = false;
    }
}

PauseMenu::Result PauseMenu::UpdateQuitConfirm(
    int mouseX,
    int mouseY,
    bool mousePressed)
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    const int panelTop =
        screenHeight / 2 -
        CONFIRM_PANEL_HEIGHT / 2;

    const int buttonWidth = 230;
    const int buttonHeight = 72;
    const int buttonSpace = 45;

    const int totalWidth =
        buttonWidth * 2 +
        buttonSpace;

    const int yesLeft =
        screenWidth / 2 -
        totalWidth / 2;

    const int yesRight =
        yesLeft +
        buttonWidth;

    const int noLeft =
        yesRight +
        buttonSpace;

    const int noRight =
        noLeft +
        buttonWidth;

    const int buttonTop =
        panelTop + 230;

    const int buttonBottom =
        buttonTop +
        buttonHeight;

    // ========================================
    // はい
    // ========================================

    if (IsButtonClicked(
        mouseX,
        mouseY,
        mousePressed,
        yesLeft,
        buttonTop,
        yesRight,
        buttonBottom))
    {
        return Result::QuitGame;
    }

    // ========================================
    // いいえ
    // ========================================

    if (IsButtonClicked(
        mouseX,
        mouseY,
        mousePressed,
        noLeft,
        buttonTop,
        noRight,
        buttonBottom))
    {
        menuState_ =
            MenuState::Main;

        return Result::None;
    }

    return Result::None;
}

void PauseMenu::UpdateVolumeSlider(
    int mouseX,
    int mouseY,
    bool mouseDown,
    bool& isDragging,
    int x,
    int y,
    int width,
    int& scaleStep)
{
    const int barHeight = 18;
    const int knobWidth = 24;
    const int knobHeight = 42;

    const int knobX =
        x +
        scaleStep * width /
        MAX_SCALE_STEP;

    const int knobTop =
        y -
        knobHeight / 2 +
        barHeight / 2;

    const int knobBottom =
        knobTop +
        knobHeight;

    const bool isMouseOnSlider =
        IsMouseInside(
            mouseX,
            mouseY,
            x - knobWidth,
            knobTop,
            x + width + knobWidth,
            knobBottom
        );

    if (mouseDown &&
        isMouseOnSlider)
    {
        isDragging = true;
    }

    if (!mouseDown)
    {
        isDragging = false;
    }

    if (!isDragging)
    {
        return;
    }

    const int newKnobX =
        std::clamp(
            mouseX,
            x,
            x + width
        );

    scaleStep =
        (newKnobX - x) *
        MAX_SCALE_STEP /
        width;

    scaleStep =
        std::clamp(
            scaleStep,
            MIN_SCALE_STEP,
            MAX_SCALE_STEP
        );
}

void PauseMenu::UpdateSoundVolume()
{
    SoundManager& soundManager =
        SoundManager::GetInstance();

    const float bgmScale =
        static_cast<float>(
            bgmScaleStep_
            ) / 10.0f;

    const float seScale =
        static_cast<float>(
            seScaleStep_
            ) / 10.0f;

    soundManager.SetBGMVolumeScale(
        bgmScale
    );

    soundManager.SetSEVolumeScale(
        seScale
    );
}

void PauseMenu::Draw() const
{
    if (!isActive_)
    {
        return;
    }

    DrawBackground();

    if (menuState_ == MenuState::Main)
    {
        DrawMainMenu();
    }
    else if (menuState_ == MenuState::Settings)
    {
        DrawSettings();
    }
    else if (menuState_ == MenuState::QuitConfirm)
    {
        DrawQuitConfirm();
    }
}

void PauseMenu::DrawBackground() const
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    // ========================================
    // 全体を暗くする
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        185
    );

    DrawBox(
        0,
        0,
        screenWidth,
        screenHeight,
        GetColor(0, 5, 10),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // ========================================
    // 上下の暗い帯
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        130
    );

    DrawBox(
        0,
        0,
        screenWidth,
        90,
        GetColor(0, 0, 0),
        TRUE
    );

    DrawBox(
        0,
        screenHeight - 75,
        screenWidth,
        screenHeight,
        GetColor(0, 0, 0),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // ========================================
    // 走査線
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        24
    );

    for (int y = 0;
        y < screenHeight;
        y += 6)
    {
        DrawLine(
            0,
            y,
            screenWidth,
            y,
            GetColor(40, 255, 220)
        );
    }

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // ========================================
    // 画面外周
    // ========================================

    DrawBox(
        20,
        20,
        screenWidth - 20,
        screenHeight - 20,
        GetColor(20, 80, 90),
        FALSE
    );

    DrawBox(
        27,
        27,
        screenWidth - 27,
        screenHeight - 27,
        GetColor(10, 40, 50),
        FALSE
    );

    // ========================================
    // 上部ステータス
    // ========================================

    DrawLine(
        42,
        48,
        screenWidth - 42,
        48,
        GetColor(30, 145, 145),
        2
    );

    DrawString(
        55,
        58,
        "SECURITY SYSTEM // PROCESS SUSPENDED",
        GetColor(80, 205, 195)
    );

    const int now = GetNowCount();

    const int blinkAlpha =
        120 +
        static_cast<int>(
            (std::sin(
                static_cast<float>(now) *
                0.005f
            ) + 1.0f) *
            60.0f
            );

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        blinkAlpha
    );

    DrawCircle(
        screenWidth - 72,
        67,
        7,
        GetColor(60, 255, 210),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // ========================================
    // 下部表示
    // ========================================

    DrawString(
        55,
        screenHeight - 55,
        "STATUS : PAUSED",
        GetColor(70, 160, 160)
    );

    const char* rightText =
        "INPUT DEVICE : MOUSE / KEYBOARD";

    const int rightTextWidth =
        GetDrawStringWidth(
            rightText,
            -1
        );

    DrawString(
        screenWidth -
        rightTextWidth -
        55,
        screenHeight - 55,
        rightText,
        GetColor(70, 160, 160)
    );
}

void PauseMenu::DrawCyberPanel(
    int left,
    int top,
    int right,
    int bottom,
    unsigned int accentColor) const
{
    // ========================================
    // パネルの影
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        120
    );

    DrawBox(
        left + 14,
        top + 14,
        right + 14,
        bottom + 14,
        GetColor(0, 0, 0),
        TRUE
    );

    // ========================================
    // 外側の発光
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        35
    );

    DrawBox(
        left - 10,
        top - 10,
        right + 10,
        bottom + 10,
        accentColor,
        TRUE
    );

    // ========================================
    // パネル本体
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        240
    );

    DrawBox(
        left,
        top,
        right,
        bottom,
        GetColor(5, 15, 23),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 外枠
    DrawBox(
        left,
        top,
        right,
        bottom,
        accentColor,
        FALSE
    );

    // 内枠
    DrawBox(
        left + 6,
        top + 6,
        right - 6,
        bottom - 6,
        GetColor(25, 90, 105),
        FALSE
    );

    // タイトル下ライン
    DrawBox(
        left + 28,
        top + 112,
        right - 28,
        top + 115,
        accentColor,
        TRUE
    );

    // 下部ライン
    DrawBox(
        left + 28,
        bottom - 27,
        right - 28,
        bottom - 24,
        GetColor(20, 80, 90),
        TRUE
    );

    // ========================================
    // 四隅
    // ========================================

    const int cornerLength = 48;
    const int cornerThickness = 4;

    // 左上
    DrawBox(
        left,
        top,
        left + cornerLength,
        top + cornerThickness,
        accentColor,
        TRUE
    );

    DrawBox(
        left,
        top,
        left + cornerThickness,
        top + cornerLength,
        accentColor,
        TRUE
    );

    // 右上
    DrawBox(
        right - cornerLength,
        top,
        right,
        top + cornerThickness,
        accentColor,
        TRUE
    );

    DrawBox(
        right - cornerThickness,
        top,
        right,
        top + cornerLength,
        accentColor,
        TRUE
    );

    // 左下
    DrawBox(
        left,
        bottom - cornerThickness,
        left + cornerLength,
        bottom,
        accentColor,
        TRUE
    );

    DrawBox(
        left,
        bottom - cornerLength,
        left + cornerThickness,
        bottom,
        accentColor,
        TRUE
    );

    // 右下
    DrawBox(
        right - cornerLength,
        bottom - cornerThickness,
        right,
        bottom,
        accentColor,
        TRUE
    );

    DrawBox(
        right - cornerThickness,
        bottom - cornerLength,
        right,
        bottom,
        accentColor,
        TRUE
    );

    // ========================================
    // 側面の装飾
    // ========================================

    for (int i = 0; i < 6; i++)
    {
        const int decorationY =
            top + 135 + i * 42;

        DrawLine(
            left + 15,
            decorationY,
            left + 30,
            decorationY,
            GetColor(35, 120, 125),
            2
        );

        DrawLine(
            right - 30,
            decorationY,
            right - 15,
            decorationY,
            GetColor(35, 120, 125),
            2
        );
    }
}

void PauseMenu::DrawMainMenu() const
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    const int panelLeft =
        screenWidth / 2 -
        MAIN_PANEL_WIDTH / 2;

    const int panelTop =
        screenHeight / 2 -
        MAIN_PANEL_HEIGHT / 2;

    const int panelRight =
        panelLeft +
        MAIN_PANEL_WIDTH;

    const int panelBottom =
        panelTop +
        MAIN_PANEL_HEIGHT;

    const unsigned int accentColor =
        GetColor(70, 235, 215);

    DrawCyberPanel(
        panelLeft,
        panelTop,
        panelRight,
        panelBottom,
        accentColor
    );

    // ========================================
    // タイトル
    // ========================================

    DrawCenterText(
        screenWidth / 2,
        panelTop + 38,
        "SYSTEM PAUSED",
        GetColor(160, 255, 235)
    );

    DrawCenterText(
        screenWidth / 2,
        panelTop + 76,
        "GAME PROCESS HAS BEEN SUSPENDED",
        GetColor(80, 155, 160)
    );

    int mouseX = 0;
    int mouseY = 0;

    GetMousePoint(
        &mouseX,
        &mouseY
    );

    const int buttonLeft =
        screenWidth / 2 -
        MAIN_BUTTON_WIDTH / 2;

    const int buttonRight =
        buttonLeft +
        MAIN_BUTTON_WIDTH;

    const int continueTop =
        panelTop + 150;

    const int settingTop =
        continueTop +
        MAIN_BUTTON_HEIGHT +
        MAIN_BUTTON_SPACE;

    const int quitTop =
        settingTop +
        MAIN_BUTTON_HEIGHT +
        MAIN_BUTTON_SPACE;

    DrawMenuButton(
        mouseX,
        mouseY,
        buttonLeft,
        continueTop,
        buttonRight,
        continueTop + MAIN_BUTTON_HEIGHT,
        "ゲームを続ける"
    );

    DrawMenuButton(
        mouseX,
        mouseY,
        buttonLeft,
        settingTop,
        buttonRight,
        settingTop + MAIN_BUTTON_HEIGHT,
        "設定"
    );

    DrawMenuButton(
        mouseX,
        mouseY,
        buttonLeft,
        quitTop,
        buttonRight,
        quitTop + MAIN_BUTTON_HEIGHT,
        "ゲームをやめる",
        true
    );

    DrawCenterText(
        screenWidth / 2,
        panelBottom - 48,
        "[ ESC ] ゲーム画面へ戻る",
        GetColor(100, 175, 175)
    );
}

void PauseMenu::DrawSettings() const
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    const int panelLeft =
        screenWidth / 2 -
        SETTING_PANEL_WIDTH / 2;

    const int panelTop =
        screenHeight / 2 -
        SETTING_PANEL_HEIGHT / 2;

    const int panelRight =
        panelLeft +
        SETTING_PANEL_WIDTH;

    const int panelBottom =
        panelTop +
        SETTING_PANEL_HEIGHT;

    const unsigned int accentColor =
        GetColor(70, 220, 255);

    DrawCyberPanel(
        panelLeft,
        panelTop,
        panelRight,
        panelBottom,
        accentColor
    );

    // ========================================
    // タイトル
    // ========================================

    DrawCenterText(
        screenWidth / 2,
        panelTop + 38,
        "AUDIO CONFIGURATION",
        GetColor(160, 240, 255)
    );

    DrawCenterText(
        screenWidth / 2,
        panelTop + 76,
        "音量倍率を調整してください",
        GetColor(100, 170, 185)
    );

    const int sliderWidth = 560;

    const int sliderX =
        screenWidth / 2 -
        sliderWidth / 2;

    const int bgmSliderY =
        panelTop + 200;

    const int seSliderY =
        panelTop + 355;

    DrawVolumeSlider(
        sliderX,
        bgmSliderY,
        sliderWidth,
        "BGM OUTPUT",
        bgmScaleStep_
    );

    DrawVolumeSlider(
        sliderX,
        seSliderY,
        sliderWidth,
        "SE OUTPUT",
        seScaleStep_
    );

    int mouseX = 0;
    int mouseY = 0;

    GetMousePoint(
        &mouseX,
        &mouseY
    );

    const int buttonWidth = 340;
    const int buttonHeight = 70;

    const int buttonLeft =
        screenWidth / 2 -
        buttonWidth / 2;

    const int buttonTop =
        panelTop + 485;

    DrawMenuButton(
        mouseX,
        mouseY,
        buttonLeft,
        buttonTop,
        buttonLeft + buttonWidth,
        buttonTop + buttonHeight,
        "設定を保存して戻る"
    );

    DrawCenterText(
        screenWidth / 2,
        panelBottom - 43,
        "[ ESC ] ひとつ前の画面へ戻る",
        GetColor(100, 165, 175)
    );
}

void PauseMenu::DrawQuitConfirm() const
{
    const int screenWidth =
        Application::SCREEN_SIZE_X;

    const int screenHeight =
        Application::adjustedSizeY_;

    const int panelLeft =
        screenWidth / 2 -
        CONFIRM_PANEL_WIDTH / 2;

    const int panelTop =
        screenHeight / 2 -
        CONFIRM_PANEL_HEIGHT / 2;

    const int panelRight =
        panelLeft +
        CONFIRM_PANEL_WIDTH;

    const int panelBottom =
        panelTop +
        CONFIRM_PANEL_HEIGHT;

    const unsigned int warningColor =
        GetColor(255, 80, 80);

    DrawCyberPanel(
        panelLeft,
        panelTop,
        panelRight,
        panelBottom,
        warningColor
    );

    // ========================================
    // 警告アイコン
    // ========================================

    const int warningCenterX =
        screenWidth / 2;

    const int warningCenterY =
        panelTop + 70;

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        75
    );

    DrawCircle(
        warningCenterX,
        warningCenterY,
        39,
        warningColor,
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    DrawCircle(
        warningCenterX,
        warningCenterY,
        31,
        GetColor(75, 15, 20),
        TRUE
    );

    DrawCircle(
        warningCenterX,
        warningCenterY,
        31,
        warningColor,
        FALSE,
        3
    );

    DrawCenterText(
        warningCenterX,
        warningCenterY - 16,
        "!",
        GetColor(255, 225, 225)
    );

    // ========================================
    // 確認文章
    // ========================================

    DrawCenterText(
        screenWidth / 2,
        panelTop + 125,
        "本当にゲームを終了しますか？",
        GetColor(255, 240, 240)
    );

    DrawCenterText(
        screenWidth / 2,
        panelTop + 166,
        "保存されていない進行状況は失われます",
        GetColor(220, 145, 145)
    );

    int mouseX = 0;
    int mouseY = 0;

    GetMousePoint(
        &mouseX,
        &mouseY
    );

    // ========================================
    // ボタン
    // ========================================

    const int buttonWidth = 230;
    const int buttonHeight = 72;
    const int buttonSpace = 45;

    const int totalWidth =
        buttonWidth * 2 +
        buttonSpace;

    const int yesLeft =
        screenWidth / 2 -
        totalWidth / 2;

    const int noLeft =
        yesLeft +
        buttonWidth +
        buttonSpace;

    const int buttonTop =
        panelTop + 230;

    DrawMenuButton(
        mouseX,
        mouseY,
        yesLeft,
        buttonTop,
        yesLeft + buttonWidth,
        buttonTop + buttonHeight,
        "はい、終了する",
        true
    );

    DrawMenuButton(
        mouseX,
        mouseY,
        noLeft,
        buttonTop,
        noLeft + buttonWidth,
        buttonTop + buttonHeight,
        "いいえ、戻る"
    );

    DrawCenterText(
        screenWidth / 2,
        panelBottom - 45,
        "[ ESC ] 終了をキャンセル",
        GetColor(180, 135, 135)
    );
}

void PauseMenu::DrawVolumeSlider(
    int x,
    int y,
    int width,
    const char* label,
    int scaleStep) const
{
    const int barHeight = 18;
    const int knobWidth = 24;
    const int knobHeight = 42;

    const int barLeft = x;
    const int barTop = y;
    const int barRight = x + width;
    const int barBottom = y + barHeight;

    const int knobX =
        barLeft +
        scaleStep * width /
        MAX_SCALE_STEP;

    const int knobLeft =
        knobX -
        knobWidth / 2;

    const int knobTop =
        y -
        knobHeight / 2 +
        barHeight / 2;

    const int knobRight =
        knobX +
        knobWidth / 2;

    const int knobBottom =
        knobTop +
        knobHeight;

    const float displayScale =
        static_cast<float>(
            scaleStep
            ) / 10.0f;

    // ========================================
    // ラベル
    // ========================================

    DrawString(
        x,
        y - 68,
        label,
        GetColor(180, 235, 240)
    );

    // 倍率表示用ボックス
    DrawBox(
        x + width - 135,
        y - 75,
        x + width,
        y - 37,
        GetColor(12, 35, 48),
        TRUE
    );

    DrawBox(
        x + width - 135,
        y - 75,
        x + width,
        y - 37,
        GetColor(55, 145, 155),
        FALSE
    );

    DrawFormatString(
        x + width - 112,
        y - 68,
        GetColor(150, 255, 235),
        "%.1f x",
        displayScale
    );

    // ========================================
    // スライダー背景
    // ========================================

    DrawBox(
        barLeft,
        barTop,
        barRight,
        barBottom,
        GetColor(15, 35, 45),
        TRUE
    );

    // 現在値
    DrawBox(
        barLeft,
        barTop,
        knobX,
        barBottom,
        GetColor(45, 190, 180),
        TRUE
    );

    // 現在値の発光
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        55
    );

    DrawBox(
        barLeft,
        barTop - 5,
        knobX,
        barBottom + 5,
        GetColor(70, 255, 225),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    // 枠
    DrawBox(
        barLeft,
        barTop,
        barRight,
        barBottom,
        GetColor(70, 165, 170),
        FALSE
    );

    // ========================================
    // 目盛り
    // ========================================

    for (int i = 0;
        i <= MAX_SCALE_STEP;
        i++)
    {
        const int markX =
            barLeft +
            i * width /
            MAX_SCALE_STEP;

        const int markHeight =
            i % 5 == 0
            ? 12
            : 6;

        const unsigned int markColor =
            i == NORMAL_SCALE_STEP
            ? GetColor(120, 255, 225)
            : GetColor(55, 105, 110);

        DrawLine(
            markX,
            barBottom + 5,
            markX,
            barBottom + 5 + markHeight,
            markColor
        );
    }

    // 0.0表示
    DrawString(
        barLeft - 12,
        barBottom + 25,
        "0.0",
        GetColor(80, 140, 145)
    );

    // 1.0表示
    const int normalX =
        barLeft +
        NORMAL_SCALE_STEP *
        width /
        MAX_SCALE_STEP;

    DrawString(
        normalX - 22,
        barBottom + 25,
        "1.0",
        GetColor(110, 210, 195)
    );

    // 2.0表示
    DrawString(
        barRight - 30,
        barBottom + 25,
        "2.0",
        GetColor(80, 140, 145)
    );

    // ========================================
    // つまみ
    // ========================================

    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        65
    );

    DrawBox(
        knobLeft - 6,
        knobTop - 6,
        knobRight + 6,
        knobBottom + 6,
        GetColor(80, 255, 225),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    DrawBox(
        knobLeft,
        knobTop,
        knobRight,
        knobBottom,
        GetColor(210, 255, 245),
        TRUE
    );

    DrawBox(
        knobLeft,
        knobTop,
        knobRight,
        knobBottom,
        GetColor(40, 150, 155),
        FALSE
    );

    DrawLine(
        knobX,
        knobTop + 6,
        knobX,
        knobBottom - 6,
        GetColor(30, 100, 110),
        2
    );
}

void PauseMenu::DrawMenuButton(
    int mouseX,
    int mouseY,
    int left,
    int top,
    int right,
    int bottom,
    const char* text,
    bool isWarning) const
{
    const bool isHovered =
        IsMouseInside(
            mouseX,
            mouseY,
            left,
            top,
            right,
            bottom
        );

    unsigned int normalColor;
    unsigned int hoverColor;
    unsigned int accentColor;
    unsigned int borderColor;

    if (isWarning)
    {
        normalColor =
            GetColor(55, 25, 32);

        hoverColor =
            GetColor(125, 35, 42);

        accentColor =
            GetColor(255, 90, 90);

        borderColor =
            GetColor(155, 55, 60);
    }
    else
    {
        normalColor =
            GetColor(18, 38, 52);

        hoverColor =
            GetColor(18, 95, 110);

        accentColor =
            GetColor(100, 255, 225);

        borderColor =
            GetColor(45, 120, 130);
    }

    if (isHovered)
    {
        // 発光部分
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            50
        );

        DrawBox(
            left - 8,
            top - 8,
            right + 8,
            bottom + 8,
            accentColor,
            TRUE
        );

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0
        );

        DrawBox(
            left,
            top,
            right,
            bottom,
            hoverColor,
            TRUE
        );

        // 左側アクセント
        DrawBox(
            left,
            top,
            left + 8,
            bottom,
            accentColor,
            TRUE
        );
    }
    else
    {
        DrawBox(
            left,
            top,
            right,
            bottom,
            normalColor,
            TRUE
        );

        DrawBox(
            left,
            top,
            left + 5,
            bottom,
            borderColor,
            TRUE
        );
    }

    DrawBox(
        left,
        top,
        right,
        bottom,
        isHovered
        ? accentColor
        : borderColor,
        FALSE
    );

    // 下側の装飾ライン
    DrawLine(
        left + 25,
        bottom - 10,
        right - 25,
        bottom - 10,
        isHovered
        ? accentColor
        : borderColor
    );

    const int textWidth =
        GetDrawStringWidth(
            text,
            -1
        );

    const int textHeight =
        GetFontSize();

    const int textX =
        (left + right) / 2 -
        textWidth / 2;

    const int textY =
        (top + bottom) / 2 -
        textHeight / 2;

    DrawString(
        textX,
        textY,
        text,
        isHovered
        ? GetColor(240, 255, 250)
        : GetColor(190, 220, 220)
    );

    if (isHovered)
    {
        DrawString(
            right - 42,
            textY,
            ">",
            accentColor
        );
    }
}

bool PauseMenu::IsButtonClicked(
    int mouseX,
    int mouseY,
    bool mousePressed,
    int left,
    int top,
    int right,
    int bottom) const
{
    return
        mousePressed &&
        IsMouseInside(
            mouseX,
            mouseY,
            left,
            top,
            right,
            bottom
        );
}

void PauseMenu::DrawCenterText(
    int centerX,
    int y,
    const char* text,
    unsigned int color) const
{
    const int textWidth =
        GetDrawStringWidth(
            text,
            -1
        );

    DrawString(
        centerX -
        textWidth / 2,
        y,
        text,
        color
    );
}

bool PauseMenu::IsMouseInside(
    int mouseX,
    int mouseY,
    int left,
    int top,
    int right,
    int bottom) const
{
    return
        mouseX >= left &&
        mouseX <= right &&
        mouseY >= top &&
        mouseY <= bottom;
}

void PauseMenu::Open()
{
    isActive_ = true;
    menuState_ = MenuState::Main;

    isDraggingBGM_ = false;
    isDraggingSE_ = false;

    // 現在の倍率を取得
    // 開いただけでは音量を変更しない
    SoundManager& soundManager =
        SoundManager::GetInstance();

    bgmScaleStep_ =
        static_cast<int>(
            soundManager.GetBGMVolumeScale() *
            10.0f +
            0.5f
            );

    seScaleStep_ =
        static_cast<int>(
            soundManager.GetSEVolumeScale() *
            10.0f +
            0.5f
            );

    bgmScaleStep_ =
        std::clamp(
            bgmScaleStep_,
            MIN_SCALE_STEP,
            MAX_SCALE_STEP
        );

    seScaleStep_ =
        std::clamp(
            seScaleStep_,
            MIN_SCALE_STEP,
            MAX_SCALE_STEP
        );

    SetMousePoint(
        Application::SCREEN_SIZE_X / 2,
        Application::adjustedSizeY_ / 2
    );
}

void PauseMenu::Close()
{
    isActive_ = false;
    menuState_ = MenuState::Main;

    isDraggingBGM_ = false;
    isDraggingSE_ = false;
}

bool PauseMenu::IsActive() const
{
    return isActive_;
}

float PauseMenu::GetBgmVolumeScale() const
{
    return
        static_cast<float>(
            bgmScaleStep_
            ) / 10.0f;
}

float PauseMenu::GetSeVolumeScale() const
{
    return
        static_cast<float>(
            seScaleStep_
            ) / 10.0f;
}