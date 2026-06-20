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
};