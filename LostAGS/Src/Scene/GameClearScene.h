#pragma once
#include "SceneBase.h"

class GameClearScene : public SceneBase
{
public:
    GameClearScene(void);
    ~GameClearScene(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
private:

};