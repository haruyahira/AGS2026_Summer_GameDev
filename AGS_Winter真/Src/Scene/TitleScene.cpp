#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Object/Common/Transform.h"
#include "../Object/Common/AnimationController.h"
#include "../Manager/SoundManager.h"
#include "../Application.h"
#include "TitleScene.h"



TitleScene::TitleScene(void)
	:
	imgTitle_(-1),
	imgPushSpace_(-1),
	SceneBase()
{

}

TitleScene::~TitleScene(void)
{

}

void TitleScene::Init(void)
{
	// サウンド
	SoundManager::GetInstance()->Load();
	SoundManager::GetInstance()->PlayBGM("op");
	SoundManager::GetInstance()->SetVolumeBGM(200);

	// タイトル画像の読み込み
	imgTitle_ = resMng_.Load(ResourceManager::SRC::TITLE).handleId_;
	imgTitle2560_ = resMng_.Load(ResourceManager::SRC::TITLE_2560).handleId_;

	// PushSpace画像の読み込み
	imgPushSpace_ = resMng_.Load(ResourceManager::SRC::PUSH_SPACE).handleId_;

	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

}

void TitleScene::Update(void)
{
	if (SceneManager::GetInstance().IsPause())
	{
		return;
	}

	// シーン遷移
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
	}

}

void TitleScene::Draw(void)
{
	// 画面モード設定
    // モニターの解像度を取得
	int screenX = 0;
	int screenY = 0;
	GetDefaultState(&screenX, &screenY, NULL);
	// タイトル画像の描画
	if (screenX == 2560) {
		DrawGraph(0, 0, imgTitle2560_, true);
	}
	else {
		DrawGraph(0, 0, imgTitle_, true);
	}
	// プッシュスペース画像の描画
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 1.5,
		1.0f,0.0f,imgPushSpace_,true);

}

void TitleScene::Release(void)
{

}
