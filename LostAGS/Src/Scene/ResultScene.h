#pragma once

#include "../Manager/SceneManager.h"

// プロジェクトで使用している
// シーン基底クラスをincludeしてください
#include "SceneBase.h"

class ResultScene : public SceneBase
{
private:
    //---------------------------------
    // 結果発表の段階
    //---------------------------------
    enum class STEP
    {
        TITLE_IN,
        MONEY_COUNT,
        MONEY_FIX,
        NAME_INPUT,
        RANKING_SHOW,
        WAIT_INPUT,
        FINISH
    };

public:
    ResultScene(void);
    virtual ~ResultScene(void);

    void Init(void) override;
    void OnLoaded(void) override;

    void Update(void) override;
    void Draw(void) override;

private:
    //---------------------------------
    // 各段階の更新
    //---------------------------------
    void UpdateTitleIn(void);
    void UpdateMoneyCount(void);
    void UpdateMoneyFix(void);
    void UpdateRankingShow(void);
    void UpdateWaitInput(void);
    void UpdateFinish(void);

    //---------------------------------
    // 描画
    //---------------------------------
    void DrawBackground(void);
    void DrawTitle(void);
    void DrawMoney(void);
    void DrawRanking(void);
    void DrawNextGuide(void);

    //---------------------------------
    // 金額文字列作成
    //---------------------------------
    void MakeMoneyText(
        char* destination,
        int destinationSize,
        int money
    ) const;


    void DrawNameInput(void);
    

private:
    STEP step_;

    //---------------------------------
    // 演出用時間
    //---------------------------------
    float stepTimer_;
    float totalTimer_;

    //---------------------------------
    // 最終金額
    //---------------------------------
    int finalMoney_;

    //---------------------------------
    // 表示用金額
    //---------------------------------
    float displayMoney_;

    //---------------------------------
    // 今回の順位
    //---------------------------------
    int currentRank_;

    //---------------------------------
    // 表示済みランキング数
    //---------------------------------
    int visibleRankingCount_;

    //---------------------------------
    // ランキング登録済みか
    //---------------------------------
    bool isRankingRegistered_;

    //---------------------------------
    // 入力受付
    //---------------------------------
    bool canInput_;

    //---------------------------------
    // 金額確定時のフラッシュ
    //---------------------------------
    float flashAlpha_;

    //---------------------------------
    // 金額の拡大率
    //---------------------------------
    float moneyScale_;

    //---------------------------------
    // フォント
    //---------------------------------
    int titleFontHandle_;
    int moneyFontHandle_;
    int rankingFontHandle_;
    int guideFontHandle_;

    private:
        //---------------------------------
        // 名前入力ハンドル
        //---------------------------------
        int nameInputHandle_;

        //---------------------------------
        // 入力された名前
        //---------------------------------
        std::string playerName_;

        //---------------------------------
        // 名前登録済みか
        //---------------------------------
        bool isNameRegistered_;

        //---------------------------------
        // 名前入力用フォント
        //---------------------------------
        int nameFontHandle_;
};