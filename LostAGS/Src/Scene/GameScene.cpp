#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Shader/PostEffect/PostEffect.h"
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

	// スカイドーム
	/*skyDome_ = new SkyDome(player_->GetTransform());
	skyDome_->Init();*/

	//SetUsePerPixelLighting(TRUE);


	// ポストエフェクト
	postEffect_ = std::make_unique<PostEffect>();

	postEffect_->Init(
		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_,
		Application::PATH_SHADER);

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
	// Sound
	auto& snd = SoundManager::GetInstance();

	snd.SetBGMVolume(180);

	snd.PlayBGM(SoundManager::BGM::GAME, true);
}

void GameScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
	if (ins.IsTrgDown(KEY_INPUT_3))
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMECLEAR);
	}
	if (ins.IsTrgDown(KEY_INPUT_4))
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
	}

	postEffect_->Select({ PostEffect::TYPE::GAMING });

	HpManager::GetInstance().Update();

	stage_->Update();

	player_->Update();

	enemyMng_->Update(player_);

	// ゲームオーバー条件プレイヤーのHPが０または目標金額達成出来なかったとき

	if (player_->IsDead())
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
		return;
	}

	// 時間
	static float time = 0;

	time += 0.15f;

	// 使用中のエフェクトだけ更新
	postEffect_->Update(time);


}

void GameScene::Draw(void)
{

	// ステージの描画
	stage_->Draw();

	// プレイヤーの描画
	player_->Draw();
	
	// 敵の描画
	enemyMng_->Draw();

	//int mainScreen = SceneManager::GetInstance().GetMainScreen();

	//// ポストエフェクト描画
	//postEffect_->Draw(mainScreen);
}

void GameScene::DrawPostEffect(int mainScreen)
{
	if (postEffect_ == nullptr)
	{
		DrawGraph(0, 0, mainScreen, FALSE);
		return;
	}

	// 2Dポストエフェクト用に状態をリセット
	SetUseVertexShader(-1);
	SetUsePixelShader(-1);

	SetUseTextureToShader(0, -1);
	SetUseTextureToShader(1, -1);
	SetUseTextureToShader(2, -1);

	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);
	SetUseLighting(FALSE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// mainScreen に対してポストエフェクトをかける
	postEffect_->Draw(mainScreen);

	// 次の描画に備えて戻す
	SetUseLighting(TRUE);
	SetUseBackCulling(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}