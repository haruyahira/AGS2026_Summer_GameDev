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
	driftX = 0.0f;
}

TitleScene::~TitleScene(void)
{
	/*delete skyDome_;
	delete animationController_;*/
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
	imgTitleSelectBright3_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT3).handleId_; int imgW, imgH;
	GetGraphSize(imgTitleRedpanda_, &imgW, &imgH);

	//// 背景
	//spaceDomeTran_.pos = AsoUtility::VECTOR_ZERO;
	//skyDome_ = new SkyDome(spaceDomeTran_);
	//skyDome_->Init();

	float size;

	//// メイン惑星
	//planet_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::FALL_PLANET));
	//planet_.pos = AsoUtility::VECTOR_ZERO;
	//planet_.scl = AsoUtility::VECTOR_ONE;
	//planet_.Update();

	//// 回転する惑星
	//movePlanet_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::LAST_PLANET));
	//movePlanet_.pos = { -250.0f, -100.0f, -100.0f };
	//size = 0.7f;
	//movePlanet_.scl = { size, size, size };
	//movePlanet_.quaRotLocal = Quaternion::Euler(
	//	AsoUtility::Deg2RadF(90.0f), 0.0f, 0.0f);
	//movePlanet_.Update();

	//// キャラ
	//charactor_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));
	//charactor_.pos = { -250.0f, -32.0f, -105.0f };
	//size = 0.4f;
	//charactor_.scl = { size, size, size };
	//charactor_.quaRot = Quaternion::Euler(
	//	0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f);
	//charactor_.Update();

	//// アニメーションの設定
	//std::string path = Application::PATH_MODEL + "Player/";
	//animationController_ = new AnimationController(charactor_.modelId);
	//animationController_->Add(0, path + "Run.mv1", 20.0f);
	//animationController_->Play(0);

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
	
	// 1. 時間を進める（この値を調整すると速さが変わる）
	angleTime += 0.01f;

	currentAngle = (25.0f * DX_PI_F / 180.0f) * sinf(angleTime);

    //driftX = 20.0f * sinf(angleTime * 0.7f); // 角度と少し周期をズラすと自然

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
	int imgW, imgH;
	GetGraphSize(imgTitleRedpanda_, &imgW, &imgH);
	int pivotX = imgW / 2;
	int pivotY = imgH;
	// レッサーパンダ
	DrawRotaGraph3(
		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_ + 200,
		(int)pivotX,
		(int)pivotY,
		0.7, 0.7,
		currentAngle,
		imgTitleRedpanda_,
		true
	);
	// タイトルロゴ
	DrawExtendGraph(0, 0,Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleLogo_, true);
	// 選択肢
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelect_, true);
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelectBright1_, true);
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_, imgTitleSelectBright2_, true);
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelectBright3_, true);
	
	//DrawRotaGraph(Application::getSizeX_ / 2, 500, 1.0, 0.0, imgPush_, true);

}
