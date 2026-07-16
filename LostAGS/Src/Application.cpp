#include <DxLib.h>
#include <Windows.h>
#include <EffekseerForDXLib.h>
#include "Common/FpsController.h"
#include "Manager/InputManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SceneManager.h"
#include "Manager/SoundManager.h"
#include "Utility/DebugConsole.h"
#include "Utility/DebugMemory.h"
#include "Application.h"

//Application* Application::instance_ = nullptr;
std::unique_ptr<Application> Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_SOUND = "Data/Sound/";
const std::string Application::PATH_SHADER = "Data/Shader/";

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

	// モニターのアスペクト比によって１９２０を基準に縦のサイズを調整
	adjustedSizeY_ = (int)(SCREEN_SIZE_X * ((float)getSizeY_ / (float)getSizeX_));
	/*注意
	* UIを並べる時はadjustedSizeY_を基準のｘ％でする
	*/

	SetGraphMode(SCREEN_SIZE_X, adjustedSizeY_, 32);

	SetWindowSizeExtendRate(1.0);


#ifdef _DEBUG
	ChangeWindowMode(true);  // Debug時はウィンドウモード
#else
	ChangeWindowMode(false); // Release時はフルスクリーン
#endif




	// FPS
	fpsController_ = std::make_unique<FpsController>(168.0f);
	// デバッグコンソールスタート
	InitDebugConsole();
	DebugLog("Application Init Start\n");


	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);

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

	// サウンド管理初期化
	SoundManager::GetInstance().Init();

	// シーン管理初期化
	SceneManager::CreateInstance();

}

void Application::Run(void)
{

	auto& inputManager = InputManager::GetInstance();
	auto& sceneManager = SceneManager::GetInstance();

	int debugLogFrame = 0;

	// ゲームループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{

		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

		// 画面にもメモリ表示
		DrawDebugMemoryInfo();

		// 約1秒ごとに別コンソールへログ
		debugLogFrame++;
		if (debugLogFrame % 60 == 0)
		{
			PrintDebugMemoryInfo("Running");
		}

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


	// デバッグコンソール終了
	ReleaseDebugConsole();


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
