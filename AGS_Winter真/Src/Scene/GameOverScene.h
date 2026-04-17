#pragma once

#include "../Application.h"
#include "../Object/Common/AnimationController.h"
#include "SceneBase.h"


class Camera;

class GameOverScene : public SceneBase
{
public:
	enum REDPANDA_ANIM {
		NON,
		DANCE,
		MAX
	};

	constexpr static VECTOR INIT_MODEL_POS = { 0.0f, 0.0f, 10000.0f };	// 初期位置

	constexpr static float MOVE_SPEED = 50;	// 動く速さ

	// 画面左下に配置
	static constexpr int EXIT_BTN_W = 200;
	static constexpr int EXIT_BTN_H = 50;
	static constexpr int EXIT_BTN_X = 10;
	static constexpr int EXIT_BTN_Y = Application::SCREEN_SIZE_Y - EXIT_BTN_H - 10;


	GameOverScene();				// コンストラクタ
	~GameOverScene();				// デストラクタ

	void Init(void)			override;	// 初期化処理
	void Update(void)		override;	// 更新処理
	void Draw(void)			override;	// 描画処理
	void Release(void)		override;	// 解放処理
private:
	AnimationController* animationController_;	// アニメーションコントローラーのインスタンス
	Camera* camera_;	// カメラのインスタンス

	int modelId_;
	int animNo;
	int GoImageHandle;  // ゲームオーバー２
	int overImage;
	VECTOR modelPos_;	// モデルの位置

	int bgmHandle_;  // BGMのハンドル
	int bgmVolume_;  // BGMの音量（0?255）
	bool isBgmPaused_;  // BGMが一時停止中かどうか

	int nowInput_ = 0;	// 現在の入力状態
	int prevInput_ = 0;	// 前回の入力状態

	int fontSize;

};