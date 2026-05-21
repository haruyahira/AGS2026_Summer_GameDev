#include<DxLib.h>
#include "GameOverScene.h"
#include "TitleScene.h"
#include "SceneController.h"
#include"../Input.h"
#include"../ResourceManager.h"
#include"../Application.h"

constexpr int fade_interval = 90;

GameOverScene::GameOverScene(SceneController& ctrl) : Scene(ctrl),
update_(&GameOverScene::FadeInUpdate),
draw_(&GameOverScene::FadeDraw),
frame_(fade_interval)
{
	backH_ = ResourceManager::LoadGraph(L"Resource/img/gameover.jpg");
	gameoverStringH_ = ResourceManager::LoadGraph(L"Resource/img/gameover_string.png");
}

void GameOverScene::Update(Input& input)
{
	(this->*update_)(input);
}


void GameOverScene::FadeInUpdate(Input& input)
{
	if (--frame_ <= 0)
	{
		update_ = &GameOverScene::NormalUpdate;
		draw_ = &GameOverScene::NormalDraw;
	}
}

void GameOverScene::FadeOutUpdate(Input& input)
{
	if (++frame_ >= fade_interval)
	{
		controller_.ChangeScene(std::make_shared<TitleScene>(controller_));
		return;
	}
}

// 通常更新
void GameOverScene::NormalUpdate(Input& input)
{
	SceneController controller;
	if (input.IsTriggered("ok")) {

		update_ = &GameOverScene::FadeOutUpdate;
		draw_ = &GameOverScene::FadeDraw;
	}
}

void GameOverScene::FadeDraw()
{
	NormalDraw();

	// 黒いセロファンをアルファブレンディングしている
	auto size = Application::GetInstance().GetWindowSize();
	// 0～60→0.0～1.0に変換している
	float rate = static_cast<float>(frame_) / static_cast<float>(fade_interval);
	// 0.0～1.0*255.0→0.0～255.0に変換している
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255.0f * rate));
	// 黒セロファン
	DrawBox(0, 0, size.w, size.h, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameOverScene::Draw()
{


	(this->*draw_)();
}

void GameOverScene::NormalDraw()
{
	DrawGraph(100, 100, backH_, TRUE);
	DrawGraph(100, 100, gameoverStringH_, TRUE);
	DrawString(100, 100, L"Game Over", 0xffffff);
}
