#pragma once

#include <vector>
#include <string>
#include "SceneBase.h"
#include "../Application.h"

class CakeManager;
class Camera;

class GameClearScene : public SceneBase
{
public:
	static constexpr float BACK_SIZE_WID = 400.0f;	// 背景の大きさ
	static constexpr float ROTATION_SPEED = 250.0f;	// 回転速度
	static constexpr float ROTATION_MAX = -30.0f;	// 最大回転角度
	static constexpr float ROTATION_CHANGE_TIME = 5.0f;	// 最小回転角度
	static constexpr float BACK_MOVE_SPEED = 2000.0f;	// 移動速度
	static constexpr float BACK_ROTATION_INTARVAL = 0.1f;	// 背景

	static constexpr float ITEM_MOVE_SPEED = 10.0f;		// アイテムの移動速度
	static constexpr float ITEM_MOVE_MAX = 5.0f;		// アイテムの移動速度

	static constexpr int INDENT_SIZE = 30;	// インデントのサイズ


	GameClearScene();				// コンストラクタ
	~GameClearScene();				// デストラクタ

	void Init(void)			override;	// 初期化処理
	void Update(void)		override;	// 更新処理
	void Draw(void)			override;	// 描画処理
	void Release(void)		override;	// 解放処理
private:


	CakeManager* cake_;	// ケーキマネージャーのインスタンス
	Camera* camera_;	// カメラのインスタンス

	int clearImage;



	int fadeAlpha_ = 0;         // フェード用アルファ値（0:透明, 255:真っ黒）
	bool isFadingOut_ = false;  // フェードアウト中フラグ

	void BackUpdate();	// 背景の更新処理
	void ItemUpdate();	// アイテムの更新処理

	void BackDraw(int i);	// 背景の描画
	void ItemDraw(int i);	// アイテムの描画

	void CakeDraw();	// ケーキの描画
};

