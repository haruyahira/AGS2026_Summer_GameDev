#pragma once
#include "SceneBase.h"

class GameOverScene : public SceneBase
{
public:
    GameOverScene(void);
    ~GameOverScene(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
private:
    int selectIndex_;
    float stickInputWait_;
    int gameOverFontHandle_;
    int menuFontHandle_;
    int guideFontHandle_;
};