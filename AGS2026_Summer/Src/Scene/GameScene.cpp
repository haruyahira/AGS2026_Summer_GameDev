#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Object/Player.h"
#include "../Object/Stage.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
}

GameScene::~GameScene(void)
{
	
}

void GameScene::Init(void)
{
	//player_ = std::make_unique <Player>();
	player_ = new Player();
	player_->Init();

	// ステージ
	stage_ = new Stage(player_);
	stage_->Init();
	
	// ステージの初期設定
	stage_->ChangeStage(Stage::NAME::MAIN_PLANET);
	SceneManager::GetInstance().GetCamera()->SetFollow(&player_->GetTransform());
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FOLLOW);

}

void GameScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	

	stage_->Update();

	player_->Update();

}

void GameScene::Draw(void)
{

	stage_->Draw();

	player_->Draw();
	// ヘルプ
	DrawFormatString(840, 20, 0x000000, "移動　　：WASD");
	DrawFormatString(840, 40, 0x000000, "カメラ　：矢印キー");
	DrawFormatString(840, 60, 0x000000, "ダッシュ：右Shift");
	DrawFormatString(840, 80, 0x000000, "ジャンプ：＼(バクスラ)");

}
