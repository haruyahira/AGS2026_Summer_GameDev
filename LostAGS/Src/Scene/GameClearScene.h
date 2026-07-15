#pragma once

#include "SceneBase.h"

class GameClearScene : public SceneBase
{
public:
    GameClearScene(void);
    virtual ~GameClearScene(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:
    // 黒い背景
    void DrawBackground(void) const;

    // 全画面の一枚絵
    void DrawClearImage(void) const;

    // 中央揃えのスタッフロール
    void DrawStaffRoll(void) const;

    // リザルト画面への案内
    void DrawGuide(void) const;

    // 縁取り付き文字
    void DrawOutlinedText(
        int x,
        int y,
        const char* text,
        int textColor,
        int edgeColor,
        int fontHandle,
        int edgeSize
    ) const;

    // クレジット文字の下にラインを描画
    void DrawCreditLine(
        int x,
        int y,
        int width,
        int color
    ) const;



private:
    // 全画面に表示する一枚絵
    int clearImageHandle_;

    // フォント
    int titleFontHandle_;
    int roleFontHandle_;
    int nameFontHandle_;
    int guideFontHandle_;

    // スタッフロールの先頭座標
    float staffRollY_;

    // 1秒あたりのスクロール量
    float staffRollSpeed_;

    // シーン開始時間
    int startTime_;

    // 決定入力を受け付けるか
    bool isInputEnabled_;

    // 背景画像を動かすための時間
    float backgroundMoveTime_;

    // 背景画像の移動速度
    float backgroundMoveSpeed_;
    // 「制作・著作」が中央に到達したか
    bool isStaffRollStopped_;
};