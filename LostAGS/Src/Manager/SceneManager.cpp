#include <chrono>
#include <DxLib.h>
#include <string>
#include <cmath>
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
	sceneId_ = SCENE_ID::NONE;
#ifdef _DEBUG
	waitSceneId_ = SCENE_ID::GAME;
#else
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
			// まずポストエフェクト後のゲーム画面を描く
			gameScene->DrawPostEffect(mainScrenn_);

			// その後にUIを描く
			// これでUIはポストエフェクトに巻き込まれない
			gameScene->DrawUI();
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

	// 現在のシーンを解放
	if (scene_ != nullptr)
	{
		delete scene_;
		scene_ = nullptr;
	}

	// リソースの解放
	ResourceManager::GetInstance().Release();

	// シーンIDを変更
	sceneId_ = sceneId;

	

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


	if (scene_ != nullptr)
	{
		delete scene_;
		scene_ = nullptr;
	}

	ResourceManager::GetInstance().Release();

	

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

	const int now = GetNowCount();
	const float time = now / 1000.0f;


	auto Lerp = [](int a, int b, float t )   {
		return static_cast<int>(a + (b - a) * t);
};
	auto LerpColor = [&](int r1, int g1, int b1, int r2, int g2, int b2, float t)
		{
			return GetColor(
				Lerp(r1, r2, t),
				Lerp(g1, g2, t),
				Lerp(b1, b2, t)
			);
		};

	// =========================
	// 背景グラデーション
	// =========================
	for (int y = 0; y < screenH; y += 3)
	{
		float rate = static_cast<float>(y) / static_cast<float>(screenH);

		int color = LerpColor(
			2, 4, 8,
			10, 22, 28,
			rate
		);

		DrawBox(
			0,
			y,
			screenW,
			y + 3,
			color,
			TRUE
		);
	}

	// =========================
	// 暗いビネット風
	// =========================
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);

	for (int i = 0; i < 8; i++)
	{
		DrawBox(
			i * 18,
			i * 12,
			screenW - i * 18,
			screenH - i * 12,
			GetColor(0, 0, 0),
			FALSE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// =========================
	// 走査線
	// =========================
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 35);

	for (int y = 0; y < screenH; y += 6)
	{
		DrawLine(
			0,
			y,
			screenW,
			y,
			GetColor(0, 255, 180)
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// =========================
	// 中央パネル
	// =========================
	const int panelW = 720;
	const int panelH = 300;
	const int panelX = screenW / 2 - panelW / 2;
	const int panelY = screenH / 2 - panelH / 2;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);

	DrawBox(
		panelX,
		panelY,
		panelX + panelW,
		panelY + panelH,
		GetColor(5, 12, 16),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 外枠
	DrawBox(
		panelX,
		panelY,
		panelX + panelW,
		panelY + panelH,
		GetColor(40, 180, 160),
		FALSE
	);

	DrawBox(
		panelX + 5,
		panelY + 5,
		panelX + panelW - 5,
		panelY + panelH - 5,
		GetColor(20, 80, 90),
		FALSE
	);

	// 角の装飾
	const int corner = 45;
	const int accent = GetColor(100, 255, 220);

	DrawLine(panelX, panelY, panelX + corner, panelY, accent, 3);
	DrawLine(panelX, panelY, panelX, panelY + corner, accent, 3);

	DrawLine(panelX + panelW, panelY, panelX + panelW - corner, panelY, accent, 3);
	DrawLine(panelX + panelW, panelY, panelX + panelW, panelY + corner, accent, 3);

	DrawLine(panelX, panelY + panelH, panelX + corner, panelY + panelH, accent, 3);
	DrawLine(panelX, panelY + panelH, panelX, panelY + panelH - corner, accent, 3);

	DrawLine(panelX + panelW, panelY + panelH, panelX + panelW - corner, panelY + panelH, accent, 3);
	DrawLine(panelX + panelW, panelY + panelH, panelX + panelW, panelY + panelH - corner, accent, 3);

	// =========================
	// タイトル
	// =========================
	const char* title = "SYSTEM LOADING";
	int titleW = GetDrawStringWidth(title, strlen(title));

	DrawString(
		screenW / 2 - titleW / 2,
		panelY + 35,
		title,
		GetColor(150, 255, 230)
	);

	const char* subTitle = "Connecting security network...";
	int subTitleW = GetDrawStringWidth(subTitle, strlen(subTitle));

	DrawString(
		screenW / 2 - subTitleW / 2,
		panelY + 70,
		subTitle,
		GetColor(120, 170, 170)
	);

	// =========================
	// 回転するローディングリング
	// =========================
	const int centerX = screenW / 2;
	const int centerY = panelY + 150;

	const int ringRadius = 42;
	const int dotCount = 16;

	for (int i = 0; i < dotCount; i++)
	{
		float angle =
			time * 3.5f +
			DX_TWO_PI_F * static_cast<float>(i) / static_cast<float>(dotCount);

		float blink =
			(sinf(time * 4.0f + i * 0.5f) + 1.0f) * 0.5f;

		int alpha = 60 + static_cast<int>(blink * 180.0f);

		int x = centerX + static_cast<int>(cosf(angle) * ringRadius);
		int y = centerY + static_cast<int>(sinf(angle) * ringRadius);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		DrawCircle(
			x,
			y,
			5,
			GetColor(80, 255, 220),
			TRUE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 中央のコア
	float coreBlink = (sinf(time * 5.0f) + 1.0f) * 0.5f;
	int coreAlpha = 120 + static_cast<int>(coreBlink * 100.0f);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, coreAlpha);

	DrawCircle(
		centerX,
		centerY,
		18,
		GetColor(70, 220, 255),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawCircle(
		centerX,
		centerY,
		18,
		GetColor(180, 255, 255),
		FALSE
	);

	// =========================
	// ローディングバー
	// =========================
	const int barW = 520;
	const int barH = 18;
	const int barX = screenW / 2 - barW / 2;
	const int barY = panelY + 225;

	DrawBox(
		barX,
		barY,
		barX + barW,
		barY + barH,
		GetColor(10, 30, 35),
		TRUE
	);

	DrawBox(
		barX,
		barY,
		barX + barW,
		barY + barH,
		GetColor(50, 160, 150),
		FALSE
	);

	// 流れる光
	int sweepW = 120;
	int sweepX =
		barX - sweepW +
		static_cast<int>(fmodf(time * 260.0f, static_cast<float>(barW + sweepW)));

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);

	DrawBox(
		sweepX,
		barY + 2,
		sweepX + sweepW,
		barY + barH - 2,
		GetColor(80, 255, 220),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// =========================
	// ロード中テキスト
	// =========================
	int dotCountText = static_cast<int>((now / 350) % 4);

	char loadingText[128] = "Loading";
	for (int i = 0; i < dotCountText; i++)
	{
		strcat_s(loadingText, ".");
	}

	int loadingTextW = GetDrawStringWidth(loadingText, strlen(loadingText));

	DrawString(
		screenW / 2 - loadingTextW / 2,
		barY + 38,
		loadingText,
		GetColor(220, 255, 250)
	);

	// =========================
	// ロード数表示
	// =========================
	int loadNum = GetASyncLoadNum();

	char fileText[128];
	sprintf_s(fileText, "Now Loading Files : %d", loadNum);

	int fileTextW = GetDrawStringWidth(fileText, strlen(fileText));

	DrawString(
		screenW / 2 - fileTextW / 2,
		barY + 68,
		fileText,
		GetColor(120, 170, 170)
	);

	// =========================
	// Tips
	// =========================
	const char* tips[] =
	{
		"TIP : 足音を抑えると敵に気づかれにくい。",
		"TIP : 暗い場所では懐中電灯の使い方が重要。",
		"TIP : 5分経過後、緊急脱出モードに移行する。",
		"TIP : アイテムは持てる数に限りがある。",
		"TIP : ゴミ箱や机の下を使って敵の視線を切れる。"
	};

	int tipIndex = static_cast<int>((now / 4000) % 5);
	const char* tipText = tips[tipIndex];

	int tipTextW = GetDrawStringWidth(tipText, strlen(tipText));

	DrawString(
		screenW / 2 - tipTextW / 2,
		panelY + panelH + 28,
		tipText,
		GetColor(180, 220, 210)
	);

	// =========================
	// 下部の小さい演出テキスト
	// =========================
	const char* footer = "PLEASE WAIT...";
	int footerW = GetDrawStringWidth(footer, strlen(footer));

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);

	DrawString(
		screenW / 2 - footerW / 2,
		screenH - 50,
		footer,
		GetColor(120, 255, 220)
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// ロード画面を1回描画した
	isLoadingDrawn_ = true;
}