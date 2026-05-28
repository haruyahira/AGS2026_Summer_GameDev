#include <DxLib.h>

#include "ResultScene.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"

ResultScene::ResultScene(void)
{
    remainDay_ = 0;
    stolenMoney_ = 0;
    totalMoney_ = 0;
}

ResultScene::~ResultScene(void)
{
}

void ResultScene::Init(void)
{
    SceneManager& sceneMng = SceneManager::GetInstance();

    remainDay_ = sceneMng.GetResultRemainDay();
    stolenMoney_ = sceneMng.GetResultStolenMoney();
    totalMoney_ = sceneMng.GetResultTotalMoney();

    SetMouseDispFlag(TRUE);
    InputManager::GetInstance().SetFixMouse(false);
}
void ResultScene::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    if (ins.IsTrgDown(KEY_INPUT_RETURN))
    {
        if (SceneManager::GetInstance().CanGoNextDay())
        {
            // 次の日へ
            SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
        }
        else
        {
            // 残り0日ならタイトルへ
            SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
        }
    }
}
void ResultScene::Draw(void)
{
    int white = GetColor(255, 255, 255);
    int yellow = GetColor(255, 230, 80);
    int cyan = GetColor(100, 255, 255);
    int red = GetColor(255, 100, 100);

    DrawString(
        260,
        100,
        "RESULT",
        yellow
    );

    DrawFormatString(
        220,
        170,
        white,
        "残り%d日",
        remainDay_
    );

    DrawFormatString(
        220,
        220,
        white,
        "今回盗んだ金額：%d円",
        stolenMoney_
    );

    DrawFormatString(
        220,
        270,
        cyan,
        "合計金額：%d円",
        totalMoney_
    );

    if (remainDay_ > 0)
    {
        DrawString(
            220,
            350,
            "次の日に続く",
            yellow
        );

        DrawString(
            220,
            390,
            "Enter：次の日へ",
            white
        );
    }
    else
    {
        DrawString(
            220,
            350,
            "すべての日程が終了しました",
            red
        );

        DrawString(
            220,
            390,
            "Enter：タイトルへ戻る",
            white
        );
    }
}