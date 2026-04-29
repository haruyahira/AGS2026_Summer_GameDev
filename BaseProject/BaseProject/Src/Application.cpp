#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Common/FpsController.h"
#include "Manager/InputManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SceneManager.h"
#include "Application.h"

//Application* Application::instance_ = nullptr;
std::unique_ptr<Application> Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		//instance_ = new Application();
		//instance_ = std::make_unique<Application>();
		instance_.reset(new Application());
		/*
		* 学習メモ
		* make_uniqueはここではクラスの外側からNEWできないので上の対処法
		* 作ったインスタンスがunique_pterに移している
		*/
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{

	SetWindowText("AGS2026_Summer");

	// ウィンドウサイズ
	GetDefaultState(&getSizeX_, &getSizeY_, NULL, &getFps_); // 情報取得
	ChangeWindowMode(false); // ウィンドウモード
	SetGraphMode(getSizeX_, getSizeY_, 32); // 
	

	// FPS
	//fpsController_ = new FpsController(getFps_);
	fpsController_ = std::make_unique<FpsController>(getFps_);

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

	// キー制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();

}

void Application::Run(void)
{

	auto& inputManager = InputManager::GetInstance();
	auto& sceneManager = SceneManager::GetInstance();

	// ゲームループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{

		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

		// FPS描画
		fpsController_->Draw();

		ScreenFlip();

		// 理想FPS経過待ち
		fpsController_->Wait();


	}

}

void Application::Destroy(void)
{

	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	SceneManager::GetInstance().Destroy();

	// Effekseerを終了する。
	Effkseer_End();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}
	// FPS制御のメモリ解放
	//delete fpsController_;

	//delete instance_;



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
{
	isInitFail_ = false;
	isReleaseFail_ = false;
	fpsController_ = nullptr;
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
