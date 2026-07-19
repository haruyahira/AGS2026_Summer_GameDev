#pragma once

class PauseMenu
{
public:
    enum class Result
    {
        None,
        Resume,
        QuitGame
    };

private:
    enum class MenuState
    {
        Main,
        Settings,
        QuitConfirm
    };

public:
    PauseMenu();
    ~PauseMenu() = default;

    // 初期化
    void Init();

    // 更新
    Result Update();

    // 描画
    void Draw() const;

    // ポーズメニューを開く・閉じる
    void Open();
    void Close();

    // ポーズ中か
    bool IsActive() const;

    // 現在の音量倍率
    float GetBgmVolumeScale() const;
    float GetSeVolumeScale() const;

private:
    // マウスが指定範囲内にあるか
    bool IsMouseInside(
        int mouseX,
        int mouseY,
        int left,
        int top,
        int right,
        int bottom
    ) const;

    // ボタンがクリックされたか
    bool IsButtonClicked(
        int mouseX,
        int mouseY,
        bool mousePressed,
        int left,
        int top,
        int right,
        int bottom
    ) const;

    // 中央揃えで文字を描画
    void DrawCenterText(
        int centerX,
        int y,
        const char* text,
        unsigned int color
    ) const;

    // ボタン描画
    void DrawMenuButton(
        int mouseX,
        int mouseY,
        int left,
        int top,
        int right,
        int bottom,
        const char* text,
        bool isWarning = false
    ) const;

    // 音量倍率スライダー更新
    void UpdateVolumeSlider(
        int mouseX,
        int mouseY,
        bool mouseDown,
        bool& isDragging,
        int x,
        int y,
        int width,
        int& scaleStep
    );

    // 音量倍率スライダー描画
    void DrawVolumeSlider(
        int x,
        int y,
        int width,
        const char* label,
        int scaleStep
    ) const;

    // メインメニュー更新
    Result UpdateMainMenu(
        int mouseX,
        int mouseY,
        bool mousePressed
    );

    // 設定画面更新
    void UpdateSettings(
        int mouseX,
        int mouseY,
        bool mouseDown,
        bool mousePressed
    );

    // 終了確認画面更新
    Result UpdateQuitConfirm(
        int mouseX,
        int mouseY,
        bool mousePressed
    );

    // 音量倍率をSoundManagerへ反映
    void UpdateSoundVolume();

    // 背景描画
    void DrawBackground() const;

    // サイバー風パネル描画
    void DrawCyberPanel(
        int left,
        int top,
        int right,
        int bottom,
        unsigned int accentColor
    ) const;

    // メインメニュー描画
    void DrawMainMenu() const;

    // 設定画面描画
    void DrawSettings() const;

    // 終了確認画面描画
    void DrawQuitConfirm() const;

private:
    // ポーズメニューを表示中か
    bool isActive_;

    // 現在の画面
    MenuState menuState_;

    // 前フレームの入力
    bool previousEsc_;
    bool previousMouseLeft_;

    // スライダーのドラッグ状態
    bool isDraggingBGM_;
    bool isDraggingSE_;

    // 音量倍率の段階
    // 0  = 0.0倍
    // 10 = 1.0倍
    // 20 = 2.0倍
    int bgmScaleStep_;
    int seScaleStep_;
};