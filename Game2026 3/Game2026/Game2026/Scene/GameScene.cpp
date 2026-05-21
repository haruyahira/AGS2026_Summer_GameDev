#include<DxLib.h>
#include "GameOverScene.h"
#include "GameScene.h"
#include "SceneController.h"
#include"../Input.h"
#include"../ResourceManager.h"
#include"../Application.h"
#include"PauseScene.h"

constexpr int fade_interval = 30;
GameScene::GameScene(SceneController& ctrl) : Scene(ctrl),
update_(&GameScene::FadeInUpdate),
draw_(&GameScene::FadeDraw),	
frame_(fade_interval),
deadBullet_(true)
{
	const auto& wsize = Application::GetInstance().GetWindowSize();
	pos_ = { wsize.w * 0.5f, wsize.h* 0.5f };
	vel_ = {};
	gameH_ = ResourceManager::LoadGraph(L"Resource/img/gaming.png");
}
void GameScene::Update(Input& input)
{
	(this->*update_)(input);
}

void GameScene::Draw()
{
	(this->*draw_)();
}

/// <summary>

void GameScene::FadeInUpdate(Input& input)
{
	if (--frame_ <= 0)
	{
		update_ = &GameScene::NormalUpdate;
		draw_ = &GameScene::NormalDraw;
	}
}

void GameScene::FadeOutUpdate(Input& input)
{
	if (++frame_ >= fade_interval)
	{
		controller_.ChangeScene(std::make_shared<GameOverScene>(controller_));
		return;
	}
}

// 通常更新
void GameScene::NormalUpdate(Input& input)
{
	SceneController controller;
	if (input.IsTriggered("ok")) {

		update_ = &GameScene::FadeOutUpdate;
		draw_ = &GameScene::FadeDraw;
		return;
	}
	if (input.IsTriggered("pause")) {
		controller_.PushScene(std::make_shared<PauseScene>(controller_));
		return;
	}
	constexpr float speed = 0.5f;
	Vector2 vel = {};
	if (input.IsPressed("up")) {
		vel.y = -1.0f;
	}	
	if (input.IsPressed("down")) {
		vel.y = 1.0f;
	
	}
	if (input.IsPressed("left")) {
		vel.x= -1.0f;
	}

	if (input.IsPressed("right")) {
		vel.x = 1.0f;
	}

	constexpr float jump_power = -5.0f;
	constexpr float gravity = 0.25f;
	if (input.IsTriggered("jump")) {
		isJumping_ = true;
		vel_ += {0.0f, jump_power};
	}
	if (isJumping_) {
		vel_ += {0.0f, gravity};
	}

	vel_ += vel.Normalied() * speed;
	pos_ += vel_;
	auto bottom = Application::GetInstance().GetWindowSize().h;
	if (bottom <= pos_.y + 20.0f) {
		isJumping_ = false;
		pos_.y = bottom - 20.0f;
		vel_.y = 0.0f;
	}
}

void GameScene::FadeDraw()
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

// 通常描画
void GameScene::NormalDraw()
{
	DrawGraph(100, 100, gameH_, TRUE);
	DrawString(100, 100, L"GameScene", 0xffffff);

	DrawCircleAA(pos_.x, pos_.y,
		10.0f, 32, 0xffaaaa);
}


