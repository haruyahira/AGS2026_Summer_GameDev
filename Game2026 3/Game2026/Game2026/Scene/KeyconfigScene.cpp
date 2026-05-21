#include "KeyconfigScene.h"
#include"../Input.h"
#include"../Application.h"
#include"SceneController.h"
#include<DxLib.h>
constexpr int expend_interval = 20;
constexpr int margin_size = 80;
void KeyconfigScene::AppearUpdate(Input&)
{
	if (++frame_ >= expend_interval) {
		update_ = &KeyconfigScene::NormalUpdate;
		draw_ = &KeyconfigScene::NormalDraw;
	}
}
void KeyconfigScene::NormalUpdate(Input& input)
{
	if (input.IsTriggered("cancel")) {
		update_ = &KeyconfigScene::DisappearUpdate;
		draw_ = &KeyconfigScene::ExpandDraw;
	}
}


void KeyconfigScene::DrawFrame(float rate)
{
	const auto& wsize = Application::GetInstance().GetWindowSize();

	const auto centerY = wsize.h / 2;// 真ん中のY座標
	auto height = (wsize.h - margin_size) / 2;// 広がり切った時の高さ/2(真ん中から見た高さ)

	// 元の画面のサイドを落とすためのセロファン

	DrawBoxAA(margin_size, centerY - height * rate,
		wsize.w - margin_size, centerY + height * rate,
		0x00aa00, true, 1.0f);


	// 枠の描画(白枠)
	DrawBoxAA(margin_size, centerY - height * rate,
		wsize.w - margin_size, centerY + height * rate,
		0xffffff, false, 3.0);
}

void KeyconfigScene::EditingUpdate(Input&)
{
}

void KeyconfigScene::DisappearUpdate(Input&)
{
	if (--frame_ <= 0) {
		controller_.PopScene();
		return;
	}
}




void KeyconfigScene::ExpandDraw()
{
	DrawFrame(static_cast<float>(frame_) / static_cast<float>(expend_interval));
}

void KeyconfigScene::NormalDraw() {
	DrawFrame(1.0f);

	constexpr int text_top = margin_size + 50;
	constexpr int text_left = margin_size + 100;
	constexpr int row_height = 30;
	int y = text_top;
	int x = text_left;
	for (auto& inputRow : input_.inputTable_) {
		DrawFormatString(x, y, 0xffffff, L"%s", inputRow.first);
		y += row_height;
	}

}



void KeyconfigScene::EditingDraw()
{

}



KeyconfigScene::KeyconfigScene(SceneController& controller, Input& input):
	Scene(controller),
	input_(input),
	frame_(0)
{
	update_ = &KeyconfigScene::AppearUpdate;
	draw_ = &KeyconfigScene::ExpandDraw;
}
void KeyconfigScene::Update(Input& input)
{
	(this->*update_)(input);
}
void KeyconfigScene::Draw()
{
	(this->*draw_)();
}