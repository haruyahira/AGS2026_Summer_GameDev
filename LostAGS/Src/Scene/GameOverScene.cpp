#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "GameOverScene.h"

GameOverScene::GameOverScene(void)
{
}

GameOverScene::~GameOverScene(void)
{
}

void GameOverScene::Init(void)
{
}

void GameOverScene::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    // Rキーでゲームをやり直し
    if (ins.IsTrgDown(KEY_INPUT_R))
    {
        SceneManager::GetInstance().ResetGameResultData();
        SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
        return;
    }

    // Tキーでタイトルへ
    if (ins.IsTrgDown(KEY_INPUT_T))
    {
        SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
        return;
    }
}

void GameOverScene::Draw(void)
{
    DrawString(
        260,
        180,
        "GAME OVER",
        GetColor(255, 0, 0)
    );

    DrawString(
        220,
        260,
        "R : RETRY",
        GetColor(255, 255, 255)
    );

    DrawString(
        220,
        300,
        "T : TITLE",
        GetColor(255, 255, 255)
    );
}