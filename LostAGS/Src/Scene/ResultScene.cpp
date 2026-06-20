#include <DxLib.h>

#include "ResultScene.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"

ResultScene::ResultScene(void)
{
    remainDay_ = 0;
    stolenMoney_ = 0;
    totalMoney_ = 0;

    targetMoney_ = 0;
    needMoney_ = 0;
    selectIndex_ = 0;
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

    targetMoney_ = sceneMng.GetTargetMoney();
    needMoney_ = sceneMng.GetNeedMoney();


    SetMouseDispFlag(TRUE);
    InputManager::GetInstance().SetFixMouse(false);
}
void ResultScene::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    if (
        ins.IsTrgDown(KEY_INPUT_SPACE) ||
        ins.IsPadBtnTrgDown(
            InputManager::JOYPAD_NO::PAD1,
            InputManager::JOYPAD_BTN::DOWN)
        )
    {

        if (SceneManager::GetInstance().CanGoNextDay())
        {
            // 次の日へ
            SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
            return;
        }

        // 最終日終了時
        if (SceneManager::GetInstance().IsGameClear())
        {
            // 一回も死んでいない && 目標金額達成
            SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMECLEAR);
            return;
        }
        else
        {
            // 条件未達成ならゲームオーバー
            SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
            return;
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
        80,
        "RESULT",
        yellow
    );

    DrawFormatString(
        220,
        140,
        white,
        "残り%d日",
        remainDay_
    );

    DrawFormatString(
        220,
        190,
        white,
        "今回盗んだ金額：%d円",
        stolenMoney_
    );

    DrawFormatString(
        220,
        240,
        cyan,
        "合計金額：%d円",
        totalMoney_
    );

    DrawFormatString(
        220,
        290,
        yellow,
        "目標金額：%d円",
        targetMoney_
    );

    if (needMoney_ > 0)
    {
        DrawFormatString(
            220,
            330,
            red,
            "目標金額まであと%d円！",
            needMoney_
        );
    }
    else
    {
        DrawString(
            220,
            330,
            "目標金額達成！",
            yellow
        );
    }

    if (remainDay_ > 0)
    {
        DrawString(
            220,
            390,
            "次の日に続く",
            yellow
        );

        DrawString(
            220,
            430,
            "Space：次の日へ",
            white
        );
    }
    else
    {
        DrawString(
            220,
            390,
            "すべての日程が終了しました",
            red
        );

        DrawString(
            220,
            430,
            "Space：結果へ",
            white
        );
    }
}