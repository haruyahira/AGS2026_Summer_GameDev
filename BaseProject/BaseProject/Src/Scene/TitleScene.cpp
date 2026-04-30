#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/SkyDome.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
{
	
	/*skyDome_ = nullptr;
	animationController_ = nullptr;*/
	angleTime = 0.0f;
	currentAngle = 0.0f;
	imgPandaX_ = 0;
	imgPandaY_ = 0;

	imgTitleBack_ = -1;
	imgTitleLogo_ = -1;
	imgTitleRedpanda_ = -1;
	imgTitleSelect_ = -1;
	imgTitleSelectBright1_ = -1;
	imgTitleSelectBright2_ = -1;
	imgTitleSelectBright3_ = -1;

}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// 画像読み込み
	imgTitleBack_ = resMng_.Load(ResourceManager::SRC::BACK_GROUND).handleId_;
	imgTitleLogo_ = resMng_.Load(ResourceManager::SRC::TITLE_LOGO).handleId_;
    imgTitleRedpanda_ = resMng_.Load(ResourceManager::SRC::TITLE_READ_PANDA).handleId_;
	imgTitleSelect_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT).handleId_;
	imgTitleSelectBright1_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT1).handleId_;
	imgTitleSelectBright2_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT2).handleId_;
	imgTitleSelectBright3_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT3).handleId_;
	
	// レッサーパンダの画像サイズを取得
	GetGraphSize(imgTitleRedpanda_, &imgPandaX_, &imgPandaY_);
	imgPandaX_ /= CENTER_DIVIDER;


	fontHandle_ = CreateFontToHandle("ＭＳ Ｐゴシック", 64, 3, DX_FONTTYPE_ANTIALIASING_8X8);
	
	// ループで自動生成
	for (int i = 0; i < messages.size(); ++i) {
		telops_.push_back({
			(float)Application::SCREEN_SIZE_X + (i * 300), // 重ならないようにずらす
			(float)(100 + i * 80),                         // 高さをずらす
			1.5f + (GetRand(20) / 10.0f),                  // スピードをランダムにする
			messages[i],
			GetColor(255, 255, 255)
			});
	}

	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

}

void TitleScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
	// レッサーパンダ
	UpdateRedpanda();

	// 全てのテロップに対して処理を行う
	for (auto& telop : telops_) {
		telop.x -= telop.speed; // 左へ移動

		// 文字の横幅を取得
		textX_ = GetDrawStringWidth(telop.text.c_str(), (int)telop.text.length());

		// 画面左端に消えたら右端に戻す
		if (telop.x < -textX_) {
			telop.x = (float)Application::SCREEN_SIZE_X;
			// 右に戻るついでに、高さをランダムに変えるとよりニコニコっぽい
			telop.y = (float)(GetRand(Application::adjustedSizeY_));
		}
	}
	//// 惑星の回転
	//movePlanet_.quaRot = movePlanet_.quaRot.Mult(
	//	Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(-1.0f)));
	//movePlanet_.Update();

	//// キャラアニメーション
	//animationController_->Update();

	//skyDome_->Update();

}

void TitleScene::Draw(void)
{

//	skyDome_->Draw();

	/*MV1DrawModel(planet_.modelId);
	MV1DrawModel(movePlanet_.modelId);
	MV1DrawModel(charactor_.modelId);*/
	
	// 背景
	DrawExtendGraph(0,0,
		Application::SCREEN_SIZE_X, Application::adjustedSizeY_, imgTitleBack_, true);
	
	// レッサーパンダ
	DrawRedpanda();

	// タイトルロゴ
	DrawExtendGraph(0, 0,Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleLogo_, true);
	// 選択肢
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelect_, true);
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelectBright1_, true);
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_, imgTitleSelectBright2_, true);
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelectBright3_, true);
	
	for (const auto& t : telops_) {
		// 縁取り文字で描画
		DrawStringFToHandle((int)t.x, (int)t.y,
			t.text.c_str(), t.color, fontHandle_);
		//DrawString((int)t.x, (int)t.y, t.text.c_str(), t.color, GetColor(0, 0, 0));
	}
}

void TitleScene::UpdateRedpanda(void)
{
	// 時間を進める（この値を調整すると速さが変わる）
	angleTime += 0.015f;

	currentAngle = (25.0f * DX_PI_F / 180.0f) * sinf(angleTime);
}

void TitleScene::DrawRedpanda(void)
{

	// レッサーパンダ
	DrawRotaGraph3(
		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_ + REDPANDA_ADJUST_Y,
		(int)imgPandaX_,
		(int)imgPandaY_,
		0.7, 0.7,
		currentAngle,
		imgTitleRedpanda_,
		true
	);
}


