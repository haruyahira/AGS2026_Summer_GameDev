#include <DxLib.h>
#include "GameOverScene.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Common/AnimationController.h"





GameOverScene::GameOverScene()
{
}

GameOverScene::~GameOverScene()
{
}


void GameOverScene::Init(void)
{
	overImage = LoadGraph((Application::PATH_IMAGE + "o.png").c_str());
}

void GameOverScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(
			SceneManager::SCENE_ID::TITLE);
	}
}

void GameOverScene::Draw(void)
{

	DrawGraph(0, 0, overImage, true);

}

void GameOverScene::Release(void)
{

	DeleteGraph(overImage);  // ゲームオーバー画像の削除

}
