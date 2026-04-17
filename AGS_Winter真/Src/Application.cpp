#include <DxLib.h>
#include <iostream>
#include <EffekseerForDXLib.h>
#include "Object/Actor/Charactor/Player.h"
#include "Manager/InputManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SceneManager.h"
#include "Manager/EffectManager.h"
#include "Common/FpsController.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_BGM = "Data/BGM/";


void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	using namespace std;
	
	// アプリケーションの初期設定
	SetWindowText("2416033_平崎晴陽");
	cout << "超かぐや姫！/n";

	// 画面モード設定
	// モニターの解像度を取得
	int screenX = 0;
	int screenY = 0;
	GetDefaultState(&screenX, &screenY, NULL);
#if 0
	SetGraphMode(SCREEN_SIZE_WID_HALF, SCREEN_SIZE_HIG_HALF, 32);
	ChangeWindowMode(true);
#else
	// フルスクリーン
	SetGraphMode(screenX, screenY, 32);

	ChangeWindowMode(false); // ウィンドウモード

#endif

	// FPS制御初期化
	fpsController_ = new FpsController(FRAME_RATE);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseerの初期化
	InitEffekseer();

	// 乱数のシード値を設定する
	DATEDATA date;

	// 現在時刻を取得する
	GetDateTime(&date);

	// 乱数の初期値を設定する
	// 設定する数値によって、ランダムの出方が変わる
	SRand(date.Year + date.Mon + date.Day + date.Hour + date.Min + date.Sec);

	// 入力制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();


	SetUseZBuffer3D(TRUE);             // Zバッファを使う
	SetWriteZBuffer3D(TRUE);           // Zバッファへの書き込みもON
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	SetUseZBufferFlag(TRUE);
	SetUseBackCulling(TRUE); // 裏面非表示（モデルが透ける防止）

	SetDrawScreen(DX_SCREEN_BACK);
	ScreenFlip();

}

void Application::Run(void)
{

	InputManager& inputManager = InputManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();

	// ゲームループ
	while (ProcessMessage() == 0 && !SceneManager::GetInstance().IsEnd())
	{

		inputManager.Update();
		sceneManager.Update();

		

		sceneManager.Draw();
	
#ifdef _DEBUG
		// 平均FPS描画
		fpsController_->Draw();
#endif // _DEBUG

		ScreenFlip();

		// 理想FPS経過待ち
		fpsController_->Wait();

	}

}

void Application::Destroy(void)
{

	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	
	// シーン管理解放
	SceneManager::GetInstance().Destroy();

	// Effekseerを終了する。
	Effkseer_End();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	// FPS制御メモリ解放
	delete fpsController_;

	// インスタンスのメモリ解放
	delete instance_;

}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
	: isInitFail_(false),
	  isReleaseFail_(false),
	  fpsController_(nullptr) 
{
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}

