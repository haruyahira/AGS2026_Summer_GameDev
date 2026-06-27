#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Object/Collider/Capsule.h"
#include "../Object/Collider/Collider.h"
#include "../Object/Common/Hp/HpManager.h"
#include "../Object/Enemy/EnemyManager.h"
#include "../Object/Stage/Stage.h"
#include "../Object/Player.h"
#include "../Object/Stage/Planet.h"
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

	enemyMng_->SetStage(stage_);

	//SetUsePerPixelLighting(TRUE);

	SceneManager::GetInstance().GetCamera()->SetFollow(&player_->GetTransform());
	
	// 初期視点設定
#ifdef _DEBUG
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIRST_PERSON);
#else
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIRST_PERSON);
#endif


	player_->Update();
	//std::weak_ptr<Camera> camera_ = SceneManager::GetInstance().GetCamera();
	Camera* camera = SceneManager::GetInstance().GetCamera();
	/*if (auto camera = camera_.lock()) {*/
		camera->Update();
	//}

		SoundManager::GetInstance().Init();
		SoundManager::GetInstance().LoadSE(
			"Walk",
			"Data/Sound/SE/Walk.mp3");
		SoundManager::GetInstance().LoadSE(
			"Run",
			"Data/Sound/SE/Walk.mp3");
		SoundManager::GetInstance().LoadSE(
			"Disc",
			"Data/Sound/SE/Discovery.wav");
		// 攻撃SE
		SoundManager::GetInstance().LoadSE(
			"Hit",
			"Data/Sound/SE/Hit.mp3");
		SoundManager::GetInstance().LoadSE(
			"Attack",
			"Data/Sound/SE/Attack.mp3");
		// BGM
		SoundManager::GetInstance().LoadBGM(
			"GameBgm",
			"Data/Sound/BGM/GameBgm.mp3");
		SoundManager::GetInstance().LoadBGM(
			"ChaseBgm",
			"Data/Sound/BGM/ChaseBgm.mp3");
		
	


		SoundManager::GetInstance().SetSESpeed("Run", 1.3f);

		SoundManager::GetInstance().PlayBGM("GameBgm");

}

void GameScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	HpManager::GetInstance().Update();

	stage_->Update();

	player_->Update();

	enemyMng_->Update(player_);

	// ゲームオーバー条件プレイヤーのHPが０または目標金額達成出来なかったとき

	if (player_->IsDead())
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
		return;
	}

	static bool wasChase = false;

	bool isChase = enemyMng_->IsAnyEnemyChasing();

	if (isChase && !wasChase)
	{
		SoundManager::GetInstance().StopAllBGM();
		SoundManager::GetInstance().PlayBGM("ChaseBgm");
	}

	if (!isChase && wasChase)
	{
		SoundManager::GetInstance().StopAllBGM();
		SoundManager::GetInstance().PlayBGM("GameBgm");
	}

	wasChase = isChase;


}

void GameScene::Draw(void)
{

	// ステージの描画
	stage_->Draw();

	// プレイヤーの描画
	player_->Draw();
	
	// 敵の描画
	enemyMng_->Draw();

}
