#include <DxLib.h>
#include <functional>
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

	step_ = STEP::INTRO;

	introTitleFont_ = -1;
	introBodyFont_ = -1;
	introSmallFont_ = -1;


}

GameScene::~GameScene(void)
{
	printf("[GameScene Destructor] called\n");

	// 敵マネージャーを先に解放
	enemyMng_.reset();

	// ステージを解放
	if (stage_ != nullptr)
	{
		delete stage_;
		stage_ = nullptr;
	}

	// プレイヤーを解放
	if (player_ != nullptr)
	{
		delete player_;
		player_ = nullptr;
	}
	if (introTitleFont_ != -1)
	{
		DeleteFontToHandle(introTitleFont_);
		introTitleFont_ = -1;
	}

	if (introBodyFont_ != -1)
	{
		DeleteFontToHandle(introBodyFont_);
		introBodyFont_ = -1;
	}

	if (introSmallFont_ != -1)
	{
		DeleteFontToHandle(introSmallFont_);
		introSmallFont_ = -1;
	}

	// ポストエフェクト解放
	postEffect_.reset();
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

	if (player_ != nullptr || stage_ != nullptr) return;
	// ポストエフェクト
	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Init(
		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_,
		Application::PATH_SHADER);

	postEffect_->Select({ PostEffect::TYPE::HORROR });


	step_ = STEP::INTRO;

	introTitleFont_ = CreateFontToHandle(NULL, 42, 3);
	introBodyFont_ = CreateFontToHandle(NULL, 26, 2);
	introSmallFont_ = CreateFontToHandle(NULL, 20, 1);


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
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FOLLOW
	);

	player_->Update();
	SceneManager::GetInstance().GetCamera()->Update();

	// BGM
	auto& snd = SoundManager::GetInstance();
	snd.SetBGMVolume(180);
	snd.PlayBGM(SoundManager::BGM::GAME, true);

}

void GameScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	// =========================
	// デバッグ用シーン遷移
	// =========================
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		return;
	}

	if (ins.IsTrgDown(KEY_INPUT_3))
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMECLEAR);
		return;
	}

	if (ins.IsTrgDown(KEY_INPUT_4))
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
		return;
	}

	// =========================
	// 導入画面
	// =========================
	if (step_ == STEP::INTRO)
	{
		if (
			ins.IsTrgDown(KEY_INPUT_RETURN) ||
			ins.IsTrgDown(KEY_INPUT_SPACE) ||
			ins.IsTrgDown(KEY_INPUT_F) ||
			ins.IsPadBtnTrgDown(
				InputManager::JOYPAD_NO::PAD1,
				InputManager::JOYPAD_BTN::DOWN)
			)
		{
			step_ = STEP::PLAY;

			// ここで脱出タイマー開始
			stage_->StartEscapeTimer();

			// 敵の非表示時間もここから開始
			hideEnemyStartTime_ = GetNowCount();
			isHideEnemyAtStart_ = true;
		}

		return;
	}

	// =========================
	// ここからゲーム本編
	// =========================
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

	if (player_->IsDead())
	{
		SoundManager::GetInstance().StopAllSound();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
		return;
	}

	static float time = 0.0f;
	time += 0.15f;

	postEffect_->Update(time);
}

void GameScene::DrawPlayerAndEnemy(void)
{
	if (enemyMng_ != nullptr)
	{
		enemyMng_->Draw();
	}

	if (player_ != nullptr)
	{
		player_->Draw();
	}
}


void GameScene::Draw(void)
{
	if (stage_ == nullptr ||
		player_ == nullptr ||
		enemyMng_ == nullptr)
	{
		return;
	}

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0
	);

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseBackCulling(TRUE);
	SetUseLighting(TRUE);

	if (isHideEnemyAtStart_)
	{
		// 敵は非表示、プレイヤーだけ描画
		stage_->Draw(
			std::bind(
				&Player::Draw,
				player_
			)
		);
	}
	else
	{
		// 敵とプレイヤーを同じ描画先へ描画
		stage_->Draw(
			std::bind(
				&GameScene::DrawPlayerAndEnemy,
				this
			)
		);
	}
}


void GameScene::DrawUI(void)
{
	if (step_ == STEP::INTRO)
	{
		DrawIntroUI();
		return;
	}

	if (stage_ != nullptr)
	{
		stage_->DrawUI();
	}

	player_->DrawUI();
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

void GameScene::DrawIntroUI(void) const
{
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 245);

	DrawBox(
		0,
		0,
		screenW,
		screenH,
		GetColor(0, 0, 0),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int boxW = screenW - 160;
	int boxH = screenH - 140;

	int boxX = screenW / 2 - boxW / 2;
	int boxY = screenH / 2 - boxH / 2;

	DrawBox(
		boxX,
		boxY,
		boxX + boxW,
		boxY + boxH,
		GetColor(90, 20, 20),
		FALSE
	);

	DrawBox(
		boxX + 6,
		boxY + 6,
		boxX + boxW - 6,
		boxY + boxH - 6,
		GetColor(160, 40, 40),
		FALSE
	);

	const char* title = "匿名の依頼";

	int titleW = GetDrawStringWidthToHandle(
		title,
		strlen(title),
		introTitleFont_
	);

	DrawStringToHandle(
		screenW / 2 - titleW / 2,
		boxY + 55,
		title,
		GetColor(255, 80, 80),
		introTitleFont_
	);

	const char* messages[] =
	{

		"指定された場所へ到着しました。",
		"",
		"価値のある物品を回収しろ。",
		"",
		"制限時間は5分。",
		"時間内に脱出装置まで戻れなければ、",
		"通常の脱出手段は停止する。",
		"",
		"これは、引き返せない危険な依頼だ。",
		"",
		"",
		"脱出する際は、初期地点にある赤い装置を殴って起動"
	};

	const int messageCount = sizeof(messages) / sizeof(messages[0]);

	int startY = boxY + 145;
	int lineHeight = 34;

	for (int i = 0; i < messageCount; i++)
	{
		int textW = GetDrawStringWidthToHandle(
			messages[i],
			strlen(messages[i]),
			introBodyFont_
		);

		DrawStringToHandle(
			screenW / 2 - textW / 2,
			startY + i * lineHeight,
			messages[i],
			GetColor(235, 235, 235),
			introBodyFont_
		);
	}

	float t = GetNowCount() / 1000.0f;
	float blink = (sinf(t * 5.0f) + 1.0f) * 0.5f;

	int color = 120 + (int)(blink * 135.0f);

	const char* startText = "ENTER / SPACE / F で開始";

	int startTextW = GetDrawStringWidthToHandle(
		startText,
		strlen(startText),
		introSmallFont_
	);

	DrawStringToHandle(
		screenW / 2 - startTextW / 2,
		boxY + boxH - 70,
		startText,
		GetColor(color, color, color),
		introSmallFont_
	);
}
