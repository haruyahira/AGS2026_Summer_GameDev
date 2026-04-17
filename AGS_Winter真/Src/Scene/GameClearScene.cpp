#include <DxLib.h>
#include "GameClearScene.h"
#include "../Application.h"
#include "../Manager/SoundManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
//#include "../Manager/Camera.h"
//#include "../Utility/AsoUtility.h"


GameClearScene::GameClearScene()
{
}

GameClearScene::~GameClearScene()
{
}

void GameClearScene::Init(void)
{
	clearImage = LoadGraph((Application::PATH_IMAGE + "c.png").c_str());

	// サウンド
	SoundManager::GetInstance()->Load();
	SoundManager::GetInstance()->PlayBGM("ed");
	SoundManager::GetInstance()->SetVolumeBGM(180);
}

void GameClearScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(
			SceneManager::SCENE_ID::TITLE);
	}
}

void GameClearScene::Draw(void)
{
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	// 画面全体にフィットさせて描画
	DrawExtendGraph(
		0,              // 左
		0,              // 上
		screenW,        // 右
		screenH,        // 下
		clearImage,     // 画像
		TRUE            // 透明色を有効
	);
}

void GameClearScene::Release(void)
{
	DeleteGraph(clearImage);
}
