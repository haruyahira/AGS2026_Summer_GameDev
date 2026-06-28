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


	isHideEnemyAtStart_ = true;
	hideEnemyStartTime_ = 0;

}

GameScene::~GameScene(void)
{
	delete player_;
	delete stage_;
}

void GameScene::Init(void)
{
	ResourceManager& res = ResourceManager::GetInstance();

	res.Load(ResourceManager::SRC::PLAYER);
	res.Load(ResourceManager::SRC::ENEMYNORMAL);
	res.Load(ResourceManager::SRC::FLOOR);
	res.Load(ResourceManager::SRC::F_TABLE);
	res.Load(ResourceManager::SRC::CEILING_LIGHT);
	res.Load(ResourceManager::SRC::WALL);
	res.Load(ResourceManager::SRC::F_F);
	res.Load(ResourceManager::SRC::F_G);
	res.Load(ResourceManager::SRC::LAPTOP);
	res.Load(ResourceManager::SRC::GAME_BGM);

	// ここでは Stage::Init() や LoadModelDuplicate はしない
}
void GameScene::OnLoaded(void)
{
	// ポストエフェクト
	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Init(
		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_,
		Application::PATH_SHADER);

	postEffect_->Select({ PostEffect::TYPE::HORROR });

	// プレイヤー
	player_ = new Player();
	player_->Init();

	// ステージ
	stage_ = new Stage(player_);
	stage_->Init();

	// ステージの初期設定
	stage_->ChangeStage(NAME::FIRST_STAGE);

	// 敵
	enemyMng_ = std::make_unique<EnemyManager>();
	enemyMng_->Init();
	enemyMng_->SetStage(stage_);

	// カメラ
	SceneManager::GetInstance().GetCamera()->SetFollow(&player_->GetTransform());
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIRST_PERSON);

	player_->Update();
	SceneManager::GetInstance().GetCamera()->Update();

	// BGM
	auto& snd = SoundManager::GetInstance();
	snd.SetBGMVolume(180);
	snd.PlayBGM(SoundManager::BGM::GAME, true);

	// 開始直後だけ敵を非表示
	hideEnemyStartTime_ = GetNowCount();
	isHideEnemyAtStart_ = true;
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


	if (isHideEnemyAtStart_)
	{
		if (GetNowCount() - hideEnemyStartTime_ >= HIDE_ENEMY_TIME)
		{
			isHideEnemyAtStart_ = false;
		}
	}


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
	if (stage_ == nullptr || player_ == nullptr || enemyMng_ == nullptr)
	{
		return;
	}

	stage_->Draw();

	player_->Draw();

	// 開始直後は敵を描かない
	if (!isHideEnemyAtStart_)
	{
		enemyMng_->Draw();
	}
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