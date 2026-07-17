#pragma once

#include <string>

#include "SceneBase.h"

class RankingScene : public SceneBase
{
public:
    RankingScene(void);
    virtual ~RankingScene(void);

    void Init(void) override;
    void OnLoaded(void) override;

    void Update(void) override;
    void Draw(void) override;

private:
    void DrawBackground(void);
    void DrawHeader(void);
    void DrawPanel(void);
    void DrawRanking(void);
    void DrawGuide(void);
    void DrawDecorations(void);

    unsigned int GetRankColor(
        int rank
    ) const;

    unsigned int GetRankBackgroundColor(
        int rank
    ) const;

    const char* GetRankName(
        int rank
    ) const;

    std::string FormatMoney(
        int money
    ) const;

    float Clamp01(
        float value
    ) const;

    float EaseOutCubic(
        float value
    ) const;

private:
    float timer_;

    bool canInput_;

    int titleFontHandle_;
    int subTitleFontHandle_;
    int rankingFontHandle_;
    int smallFontHandle_;
    int guideFontHandle_;
};