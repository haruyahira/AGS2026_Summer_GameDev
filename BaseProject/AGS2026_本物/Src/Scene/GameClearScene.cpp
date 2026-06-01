#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Application.h"
#include "GameClearScene.h"

GameClearScene::GameClearScene(void)
{
}

GameClearScene::~GameClearScene(void)
{
}

void GameClearScene::Init(void)
{
    // 必要ならここで画像やBGMを読み込む
}

void GameClearScene::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    // Rキーで最初からもう一度
    if (ins.IsTrgDown(KEY_INPUT_R))
    {
        SceneManager::GetInstance().ResetGameResultData();
        SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
        return;
    }

    // Tキーでタイトルへ戻る
    if (ins.IsTrgDown(KEY_INPUT_T))
    {
        SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
        return;
    }
}

void GameClearScene::Draw(void)
{
    // 背景
    DrawBox(
        0,
        0,
        Application::SCREEN_SIZE_X,
        Application::adjustedSizeY_,
        GetColor(10, 20, 50),
        TRUE
    );

    // タイトル文字
    DrawString(
        240,
        160,
        "GAME CLEAR!",
        GetColor(255, 255, 0)
    );

    // 説明
    DrawString(
        220,
        250,
        "R : RETRY",
        GetColor(255, 255, 255)
    );

    DrawString(
        220,
        290,
        "T : TITLE",
        GetColor(255, 255, 255)
    );
}