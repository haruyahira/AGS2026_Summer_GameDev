#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "GameOverScene.h"

GameOverScene::GameOverScene(void)
{
    selectIndex_ = 0;
    stickInputWait_ = 0.0f;
}

GameOverScene::~GameOverScene(void)
{
}

void GameOverScene::Init(void)
{
   
        SetMouseDispFlag(TRUE);

        InputManager::GetInstance().SetFixMouse(false);
 
}

void GameOverScene::Update(void)
{
    InputManager& ins =
        InputManager::GetInstance();

    SceneManager& scnMng =
        SceneManager::GetInstance();

    stickInputWait_ -= scnMng.GetDeltaTime();


    int ly =
        ins.GetPadAKeyLY(
            InputManager::JOYPAD_NO::PAD1);

    const int DEAD_ZONE = 500;

    if (stickInputWait_ <= 0.0f)
    {
        // 下
        if (
            ins.IsTrgDown(KEY_INPUT_DOWN) ||
            ly > DEAD_ZONE
            )
        {
            selectIndex_++;

            if (selectIndex_ > 1)
            {
                selectIndex_ = 0;
            }

            stickInputWait_ = 0.2f;
        }

        // 上
        if (
            ins.IsTrgDown(KEY_INPUT_UP) ||
            ly < -DEAD_ZONE
            )
        {
            selectIndex_--;

            if (selectIndex_ < 0)
            {
                selectIndex_ = 1;
            }

            stickInputWait_ = 0.2f;
        }
    }

    // 決定
    bool isDecide =
        ins.IsTrgDown(KEY_INPUT_SPACE) ||
        ins.IsPadBtnTrgDown(
            InputManager::JOYPAD_NO::PAD1,
            InputManager::JOYPAD_BTN::DOWN);

    if (!isDecide)
    {
        return;
    }

    switch (selectIndex_)
    {
    case 0:
        // Retry
        SceneManager::GetInstance().ResetGameResultData();

        SceneManager::GetInstance().ChangeScene(
            SceneManager::SCENE_ID::GAME);
        return;

    case 1:
        // Title
        SceneManager::GetInstance().ChangeScene(
            SceneManager::SCENE_ID::TITLE);
        return;
    }

    int mouseX;
    int mouseY;

    GetMousePoint(&mouseX, &mouseY);

    // RETRY
    if (
        mouseX >= 220 &&
        mouseX <= 420 &&
        mouseY >= 260 &&
        mouseY <= 290
        )
    {
        selectIndex_ = 0;

        if (ins.IsTrgMouseLeft())
        {
            SceneManager::GetInstance().ResetGameResultData();

            SceneManager::GetInstance().ChangeScene(
                SceneManager::SCENE_ID::GAME);

            return;
        }
    }

    // TITLE
    if (
        mouseX >= 220 &&
        mouseX <= 420 &&
        mouseY >= 300 &&
        mouseY <= 330
        )
    {
        selectIndex_ = 1;

        if (ins.IsTrgMouseLeft())
        {
            SceneManager::GetInstance().ChangeScene(
                SceneManager::SCENE_ID::TITLE);

            return;
        }
    }
}
void GameOverScene::Draw(void)
{
    int white = GetColor(255, 255, 255);
    int red = GetColor(255, 0, 0);
    int yellow = GetColor(255, 255, 0);

    DrawString(
        260,
        180,
        "GAME OVER",
        red
    );

    DrawString(
        220,
        260,
        selectIndex_ == 0 ?
        "> RETRY" :
        "  RETRY",
        selectIndex_ == 0 ? yellow : white
    );

    DrawString(
        220,
        300,
        selectIndex_ == 1 ?
        "> TITLE" :
        "  TITLE",
        selectIndex_ == 1 ? yellow : white
    );

    DrawString(
        220,
        360,
        "左スティック / マウス : 選択",
        white
    );

    DrawString(
        220,
        390,
        "×ボタン / 左クリック : 決定",
        white
    );
}