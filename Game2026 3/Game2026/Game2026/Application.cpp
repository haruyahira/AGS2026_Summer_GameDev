#include "Application.h"
#include <DxLib.h>
#include"Scene/SceneController.h"
#include"Scene/TitleScene.h"
#include"Input.h"

constexpr float default_width = 1280.0f;
constexpr float default_height = 720.0f;


Application::Application() : windowSize_{default_width, default_height}
{
}

Application::~Application()
{
}

Application& Application::GetInstance()
{
	static Application instance;
	return instance;
}

bool Application::Init()
{
	constexpr int display_color_bit = 32;
	SetGraphMode(windowSize_.w, windowSize_.h, display_color_bit);
	SetWindowSize(static_cast<int>(windowSize_.w),
		static_cast<int>(windowSize_.h));

	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1) {
		return false;
	}
	SetDrawScreen(DX_SCREEN_BACK);
	return true;
}

void Application::Run()
{
	Input input;
	SceneController controller;
	controller.ChangeScene(std::make_shared<TitleScene>(controller));
	while (ProcessMessage() != -1&&! isShutdownRequested) {
		ClearDrawScreen();
		input.Update();
		controller.Update(input);
		controller.Draw();
		ScreenFlip();
	}
}

void Application::Terminate()
{
	DxLib_End();
}

const Size& Application::GetWindowSize() const
{
	return windowSize_;
}

void Application::Shutdown()
{
	isShutdownRequested = true;
}
