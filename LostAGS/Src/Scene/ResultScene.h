#pragma once

#include "SceneBase.h"

class ResultScene : public SceneBase
{
public:
    ResultScene(void);
    virtual ~ResultScene(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:
    int remainDay_;
    int stolenMoney_;
    int totalMoney_;
    int targetMoney_;
    int needMoney_;
    int selectIndex_;
};