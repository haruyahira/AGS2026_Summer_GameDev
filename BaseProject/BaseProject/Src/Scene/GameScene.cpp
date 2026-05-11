#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Object/Common/Capsule.h"
#include "../Object/Common/Collider.h"
#include "../Object/SkyDome.h"
#include "../Object/Stage.h"
#include "../Object/Player.h"
#include "../Object/Planet.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
	player_ = nullptr;
	skyDome_ = nullptr;
	stage_ = nullptr;
}

GameScene::~GameScene(void)
{
	delete player_;
	delete stage_;
	delete skyDome_;
}

void GameScene::Init(void)
{

	// 全体を照らす光を極限まで暗くする
	SetLightDifColor(GetColorF(0.05f, 0.05f, 0.1f, 0.0f)); // 当たっている場所も暗い
	SetLightAmbColor(GetColorF(0.02f, 0.02f, 0.05f, 0.0f)); // 光が届かない場所はほぼ真っ暗

    // フォグの設定
	SetFogEnable(TRUE);
	SetFogColor(0, 0, 10); // フォグの色を限りなく黒に近い紺に
	SetFogStartEnd(100.0f, 1500.0f); // 1500先は完全な闇

	// プレイヤー
	player_ = new Player();
	player_->Init();

	// ステージ
	stage_ = new Stage(player_);
	stage_->Init();

	// ステージの初期設定
	stage_->ChangeStage(Stage::NAME::MAIN_PLANET);

	// スカイドーム
	skyDome_ = new SkyDome(player_->GetTransform());
	skyDome_->Init();

	SceneManager::GetInstance().GetCamera()->SetFollow(&player_->GetTransform());
	
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FOLLOW);
	//SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIRST_PERSON);

}

void GameScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	skyDome_->Update();

	stage_->Update();

	player_->Update();

}

void GameScene::Draw(void)
{
	// 全体を夜の色にする（少し青みを残して暗くする）
	SetDrawBright(50, 50, 100);

	// 背景
	skyDome_->Draw();
	stage_->Draw();
	
	player_->Draw();

	// 描画設定を元に戻す
	SetDrawBright(255, 255, 255);




	
}
