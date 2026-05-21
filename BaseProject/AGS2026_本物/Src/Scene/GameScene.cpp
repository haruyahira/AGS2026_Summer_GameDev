#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Object/Collider/Capsule.h"
#include "../Object/Collider/Collider.h"
#include "../Object/Enemy/EnemyManager.h"
#include "../Object/Stage.h"
#include "../Object/Player.h"
#include "../Object/Planet.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
	player_ = nullptr;
	stage_ = nullptr;
}

GameScene::~GameScene(void)
{
	delete player_;
	delete stage_;
}

void GameScene::Init(void)
{
	// 3Dモデルを読み込む前に、ピクセル単位のライティングを有効にする
	SetUsePixelLighting(TRUE);

	// 全体を照らす光を極限まで暗くする
	SetLightDifColor(GetColorF(0.12f, 0.12f, 0.18f, 0.0f)); // 拡散光
	SetLightAmbColor(GetColorF(0.15f, 0.15f, 0.15f, 0.0f)); // 環境光

	SetFogEnable(TRUE); // フォグを有効にする
	SetFogColor(5, 5, 15);
	SetFogStartEnd(0.0f, 1000.0f);
	SetLightEnable(FALSE); // デフォルトライトを無効にする
	// プレイヤー
	player_ = new Player();
	player_->Init();

	// 敵
	enemyMng_ = std::make_unique<EnemyManager>();
	enemyMng_->Init();

	// ステージ
	stage_ = new Stage(player_);
	stage_->Init();

	// ステージの初期設定
	stage_->ChangeStage(NAME::FIRST_STAGE);

	// スカイドーム
	/*skyDome_ = new SkyDome(player_->GetTransform());
	skyDome_->Init();*/

	//SetUsePerPixelLighting(TRUE);

	SceneManager::GetInstance().GetCamera()->SetFollow(&player_->GetTransform());
	

	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FOLLOW);
	//SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIRST_PERSON);



	player_->Update();
	//std::weak_ptr<Camera> camera_ = SceneManager::GetInstance().GetCamera();
	Camera* camera = SceneManager::GetInstance().GetCamera();
	/*if (auto camera = camera_.lock()) {*/
		camera->Update();
	//}
}

void GameScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	

	stage_->Update();

	player_->Update();

	enemyMng_->Update();

}

void GameScene::Draw(void)
{
	// 全体を夜の色にする（少し青みを残して暗くする）
	//SetDrawBright(50, 50, 100);


	stage_->Draw();
	
	player_->Draw();

	enemyMng_->Draw();

	// 描画設定を元に戻す
	//SetDrawBright(255, 255, 255);

}
