#pragma once
#include "../Object/Common/Transform.h"
#include "../Object/Common/AnimationController.h"
#include "SceneBase.h"

class TitleScene : public SceneBase
{

public:
	// 大きさ
	static constexpr VECTOR MINI_PLANET_SCL = { 0.7f, 0.7f, 0.7f }; // ミニ惑星の大きさ
	static constexpr VECTOR PLAYER_SCL = { 0.4f,0.4f,0.4f };  // プレイヤーの大きさ

	// 場所
	static constexpr VECTOR MINI_PLANET_POS = { -250.0f, -100.0f, -100.0f }; // ミニ惑星の位置
	static constexpr VECTOR PLAYER_POS = { -250.0f, -32.0f, -105.0f }; // プレイヤーの位置


	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	// 惑星
	Transform bigPlanet_;
	Transform smallPlanet_;
	// プレイヤー
	Transform player_;

	// アニメーション
	AnimationController* playerAnimCtr_;

	int imgTitle_;// タイトル画像
	int imgTitle2560_;
	int imgPushSpace_;  // プッシュスペース画像

};
