#include <chrono>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Common/Fader.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/ResultScene.h"
#include "../Scene/GameOverScene.h"
#include "../Scene/GameClearScene.h"
#include "../Manager/SoundManager.h"
#include "../Application.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

void SceneManager::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

SceneManager::SceneManager(void)
{
	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	scene_ = nullptr;
	fader_ = nullptr;

	isSceneChanging_ = false;
	isNowLoading_ = false;
	isLoadingStarted_ = false;
	isLoadingDrawn_ = false;
	loadingStartTime_ = 0;

	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;
}



void SceneManager::Init(void)
{

	gameRemainDay_ = START_REMAIN_DAY;
	resultRemainDay_ = START_REMAIN_DAY;
	resultStolenMoney_ = 0;
	resultTotalMoney_ = 0;

	waitSceneId_ = SCENE_ID::NONE;

	fader_ = new Fader();
	fader_->Init();

	// カメラ
	camera_ = new Camera();
	camera_->Init();
	
	// デルタタイム
	preTime_ = std::chrono::system_clock::now();

	// メインスクリーンの作成
	mainScrenn_ = MakeScreen(

		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_,
		TRUE);

	// 3D用の設定
	Init3D();
	// 初期シーンの設定
#ifdef _DEBUG
	sceneId_ = SCENE_ID::GAME;
	DoChangeScene(SCENE_ID::GAME);
#else
	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::TITLE;
#endif


	isSceneChanging_ = true;
	isNowLoading_ = true;
	isLoadingStarted_ = false;
	isLoadingDrawn_ = false;


	loadingStartTime_ = GetNowCount();

	fader_->SetFade(Fader::STATE::NONE);
}

void SceneManager::Init3D(void)
{

	// 背景色設定
	SetBackgroundColor(0, 0, 0);

	// Zバッファを有効にする
	SetUseZBuffer3D(true);

	// Zバッファへの書き込みを有効にする
	SetWriteZBuffer3D(true);

	// バックカリングを有効にする
	SetUseBackCulling(true);

	// ライトの設定
	SetUseLighting(true);
	
	// ライトの設定
	SetLightDifColor(GetColorF(0.0f, 0.0f, 0.0f, 0.0f));
	SetLightAmbColor(GetColorF(0.0f, 0.0f, 0.0f, 0.0f));

	// 3. フォグ（霧）の初期設定
	// ゲームシーン側で細かく制御するので、ここでは暗い色で有効化だけしておきます
	SetFogEnable(true);
	SetFogColor(0, 0, 0);
	SetFogStartEnd(200.0f, 1500.0f); // 初期値を少し手前にして闇を作りやすくする

}

void SceneManager::Update(void)
{

	// scene_ が nullptr でも、シーン遷移中なら Fade は動かしたい
	if (scene_ == nullptr && !isSceneChanging_ && !isNowLoading_)
	{
		return;
	}


	// フルスクリーン＜－＞ウィンドウの切り替え
	InputManager& ins = InputManager::GetInstance();
	// Alt + Enter を検知
	if ((ins.IsNew(KEY_INPUT_LALT) || ins.IsNew(KEY_INPUT_RALT)) && ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		// ウィンドウならtrue,フルスクリーンならfalse
	    isWindow_ = (GetWindowModeFlag() == FALSE);
		ChangeWindowMode(isWindow_);

		// 切り替え後の再設定
		SetGraphMode(Application::SCREEN_SIZE_X, Application::adjustedSizeY_, 32);

	}

	// シーンがプレイシーンなら
	if (sceneId_ == SCENE_ID::GAME)
	{
		// マウスを隠して中央に固定する
		SetMouseDispFlag(FALSE);
		InputManager::GetInstance().SetFixMouse(true);
	}
	else
	{
		// メニュー画面ならマウスを表示して自由に動かせる
		SetMouseDispFlag(TRUE);
		InputManager::GetInstance().SetFixMouse(false);
	}

	// デルタタイム
	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	fader_->Update();
	if (isSceneChanging_)
	{
		Fade();
	}
	else
	{
		if (scene_ != nullptr) {
			scene_->Update();
		}
	}

	// カメラ更新
	camera_->Update();

}

void SceneManager::Draw(void)
{
	// =========================
	// ロード中はロード画面だけ描画
	// =========================
	if (isNowLoading_)
	{
		DrawLoadingScreen();
		return;
	}

	// =========================
	// 1. mainScrenn_ にゲーム画面を描く
	// =========================
	SetDrawScreen(mainScrenn_);
	ClearDrawScreen();

	camera_->SetBeforeDraw();

	UpdateEffekseer3D();

	if (scene_ != nullptr)
	{
		scene_->Draw();
	}

	camera_->Draw();

	Effekseer_Sync3DSetting();
	DrawEffekseer3D();

	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();

	if (sceneId_ == SCENE_ID::GAME)
	{
		GameScene* gameScene = dynamic_cast<GameScene*>(scene_);

		if (gameScene != nullptr)
		{
			gameScene->DrawPostEffect(mainScrenn_);
		}
		else
		{
			DrawGraph(0, 0, mainScrenn_, FALSE);
		}
	}
	else
	{
		DrawGraph(0, 0, mainScrenn_, FALSE);
	}

	fader_->Draw();
}

void SceneManager::Destroy(void)
{

	if (scene_ != nullptr)
	{
		delete scene_;
		scene_ = nullptr;
	}

	delete fader_;
	delete camera_;

	DeleteGraph(mainScrenn_);
	delete instance_;

}

void SceneManager::ChangeScene(SCENE_ID nextId)
{
	if (isSceneChanging_ || isNowLoading_)
	{
		return;
	}

	if (sceneId_ == nextId)
	{
		return;
	}

	waitSceneId_ = nextId;

	fader_->SetFade(Fader::STATE::FADE_OUT);
	isSceneChanging_ = true;
}


void SceneManager::DoChangeScene(SCENE_ID sceneId)
{
	// 全サウンド停止
	SoundManager::GetInstance().StopAllSound();

	// リソースの解放
	ResourceManager::GetInstance().Release();

	// シーンIDを変更
	sceneId_ = sceneId;

	// 現在のシーンを解放
	if (scene_ != nullptr)
	{
		delete scene_;
		scene_ = nullptr;
	}

	// 新しいシーンを作成
	switch (sceneId_)
	{
	case SCENE_ID::TITLE:
		scene_ = new TitleScene();
		break;

	case SCENE_ID::GAME:
		scene_ = new GameScene();
		break;

	case SCENE_ID::RESULT:
		scene_ = new ResultScene();
		break;

	case SCENE_ID::GAMEOVER:
		scene_ = new GameOverScene();
		break;

	case SCENE_ID::GAMECLEAR:
		scene_ = new GameClearScene();
		break;

	default:
		scene_ = nullptr;
		break;
	}

	if (scene_ != nullptr)
	{
		scene_->Init();

		// 同期ロードの場合も、ロード完了後処理を呼ぶ
		scene_->OnLoaded();
	}

	ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;
}


SceneManager::SCENE_ID SceneManager::GetSceneID(void)
{
	return sceneId_;
}

float SceneManager::GetDeltaTime(void) const
{
	//return 1.0f / 60.0f;
	return deltaTime_;
}

Camera* SceneManager::GetCamera(void) const
{
	return camera_;
}


void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::StartAsyncChangeScene(SCENE_ID sceneId)
{
	SoundManager::GetInstance().StopAllSound();
	ResourceManager::GetInstance().Release();

	if (scene_ != nullptr)
	{
		delete scene_;
		scene_ = nullptr;
	}

	sceneId_ = sceneId;

	switch (sceneId_)
	{
	case SCENE_ID::TITLE:
		scene_ = new TitleScene();
		break;

	case SCENE_ID::GAME:
		scene_ = new GameScene();
		break;

	case SCENE_ID::RESULT:
		scene_ = new ResultScene();
		break;

	case SCENE_ID::GAMEOVER:
		scene_ = new GameOverScene();
		break;

	case SCENE_ID::GAMECLEAR:
		scene_ = new GameClearScene();
		break;

	default:
		scene_ = nullptr;
		break;
	}

	if (scene_ == nullptr)
	{
		waitSceneId_ = SCENE_ID::NONE;
		isNowLoading_ = false;
		isLoadingStarted_ = false;
		isLoadingDrawn_ = false;
		return;
	}

	// ここでは Init() しない
	// まずロード画面を表示できる状態にする
	isNowLoading_ = true;
	isLoadingStarted_ = false;
	isLoadingDrawn_ = false;
	loadingStartTime_ = GetNowCount();

	waitSceneId_ = SCENE_ID::NONE;
}

void SceneManager::BeginSceneLoad(void)
{
	if (scene_ == nullptr)
	{
		return;
	}

	// 全シーン非同期ロードにする
	SetUseASyncLoadFlag(TRUE);

	scene_->Init();

	SetUseASyncLoadFlag(FALSE);

	isLoadingStarted_ = true;
}
void SceneManager::Fade(void)
{
	if (isNowLoading_)
	{
		// ロード画面をまだ1回も描いていないなら何もしない
		if (!isLoadingDrawn_)
		{
			return;
		}

		// まだロード開始していなければ、このタイミングで開始
		if (!isLoadingStarted_)
		{
			// 起動直後など、まだ scene_ が作られていない場合
			if (scene_ == nullptr)
			{
				if (waitSceneId_ == SCENE_ID::NONE)
				{
					return;
				}

				StartAsyncChangeScene(waitSceneId_);
				return;
			}

			BeginSceneLoad();

			loadingStartTime_ = GetNowCount();

			return;
		}

		int loadNum = GetASyncLoadNum();
		int elapsedTime = GetNowCount() - loadingStartTime_;

		if (loadNum == 0 && elapsedTime >= LOADING_MIN_TIME)
		{
			if (scene_ != nullptr)
			{
				scene_->OnLoaded();
			}

			isNowLoading_ = false;
			isLoadingStarted_ = false;
			isLoadingDrawn_ = false;

			ResetDeltaTime();

			fader_->SetFade(Fader::STATE::FADE_IN);
		}

		return;
	}

	Fader::STATE fState = fader_->GetState();

	switch (fState)
	{
	case Fader::STATE::FADE_IN:
		if (fader_->IsEnd())
		{
			fader_->SetFade(Fader::STATE::NONE);
			isSceneChanging_ = false;
		}
		break;

	case Fader::STATE::FADE_OUT:
		if (fader_->IsEnd())
		{
			StartAsyncChangeScene(waitSceneId_);
			fader_->SetFade(Fader::STATE::NONE);
		}
		break;
	}
}

void SceneManager::SetResultData(int stolenMoney)
{
	// 今回盗んだ金額
	resultStolenMoney_ = stolenMoney;

	// 累計金額に加算
	resultTotalMoney_ += stolenMoney;

	// 1日終了したので残り日数を1減らす
	gameRemainDay_--;

	if (gameRemainDay_ < 0)
	{
		gameRemainDay_ = 0;
	}

	// リザルト表示用
	resultRemainDay_ = gameRemainDay_;
}
int SceneManager::GetGameRemainDay(void) const
{
	return gameRemainDay_;
}

int SceneManager::GetResultRemainDay(void) const
{
	return resultRemainDay_;
}

int SceneManager::GetResultStolenMoney(void) const
{
	return resultStolenMoney_;
}

int SceneManager::GetResultTotalMoney(void) const
{
	return resultTotalMoney_;
}

bool SceneManager::CanGoNextDay(void) const
{
	return gameRemainDay_ > 0;
}

void SceneManager::ResetGameResultData(void)
{
	gameRemainDay_ = START_REMAIN_DAY;
	resultRemainDay_ = START_REMAIN_DAY;
	resultStolenMoney_ = 0;
	resultTotalMoney_ = 0;

	// 追加：一度死んだ情報もリセット
	isPlayerDeadOnce_ = false;

}
int SceneManager::GetTargetMoney(void) const
{
	return targetMoney_;
}

int SceneManager::GetNeedMoney(void) const
{
	int needMoney = targetMoney_ - resultTotalMoney_;

	if (needMoney < 0)
	{
		needMoney = 0;
	}

	return needMoney;
}

void SceneManager::SetPlayerDeadOnce(bool isDead)
{
	isPlayerDeadOnce_ = isDead;
}

bool SceneManager::IsPlayerDeadOnce(void) const
{
	return isPlayerDeadOnce_;
}

bool SceneManager::IsGameClear(void) const
{
	// 一度でも死んでいたらクリア不可
	if (isPlayerDeadOnce_)
	{
		return false;
	}

	// 目標金額未達成ならクリア不可
	if (resultTotalMoney_ < targetMoney_)
	{
		return false;
	}

	return true;
}

int SceneManager::GetMainScreen(void)
{
	return mainScrenn_;
}

void SceneManager::DrawLoadingScreen(void)
{

	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();

	const int screenW = Application::SCREEN_SIZE_X;
	const int screenH = Application::adjustedSizeY_;

	int white = GetColor(255, 255, 255);
	int gray = GetColor(140, 140, 140);
	int blue = GetColor(80, 180, 255);

	// 背景
	DrawBox(
		0,
		0,
		screenW,
		screenH,
		GetColor(0, 0, 0),
		TRUE
	);

	int centerX = screenW / 2;
	int centerY = screenH / 2;

	// Loading のドットアニメーション
	int dotCount = static_cast<int>((GetNowCount() / 300) % 4);

	DrawString(centerX - 80, centerY - 40, "Loading", white);

	for (int i = 0; i < dotCount; i++)
	{
		DrawString(centerX + 20 + i * 16, centerY - 40, ".", white);
	}

	// 残りロード数
	int loadNum = GetASyncLoadNum();

	DrawFormatString(
		centerX - 100,
		centerY,
		gray,
		"Now Loading Files : %d",
		loadNum
	);

	// 簡単なバー背景
	int barX = centerX - 150;
	int barY = centerY + 50;
	int barW = 300;
	int barH = 16;

	DrawBox(
		barX,
		barY,
		barX + barW,
		barY + barH,
		GetColor(60, 60, 60),
		TRUE
	);

	// 動く光
	int moveX = barX + static_cast<int>((GetNowCount() / 5) % barW);

	DrawBox(
		moveX,
		barY,
		moveX + 40,
		barY + barH,
		blue,
		TRUE
	);

	// 枠
	DrawBox(
		barX,
		barY,
		barX + barW,
		barY + barH,
		white,
		FALSE
	);


	// ロード画面を1回描画した
	isLoadingDrawn_ = true;

}
