#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"
class SceneManager;
class SkyDome;
class AnimationController;

class TitleScene : public SceneBase
{

public:

	// テロップ1つ分のデータ
	struct  Telop  {
		float x, y;          // 座標
		float speed;         // 移動速度
		std::string text;    // 表示する文字
		unsigned int color;  // 文字の色
	};



	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	static constexpr float CENTER_DIVIDER = 2.0f;    // 中心計算用
	static constexpr int REDPANDA_ADJUST_Y = 200; // レッサーパンダのY座標調整用値

	void DrawRedpanda(void);
	void UpdateRedpanda(void);

	// テロップの内容一覧
	std::vector<std::string> messages = {
		"しぼあまってなに？？？ｗｗｗｗｗｗｗｗｗｗｗｗ",
		"う、おｗｗｗｗｗｗｗｗｗｗｗｗｗｗ",
		"いろＰ結婚して！！",
		"どわーｗｗｗｗｗｗｗｗｗｗｗｗ",
		"人の心とかないんか？"
	};

	// 画像
	int imgTitleBack_;
	int imgTitleLogo_;
	int imgTitleRedpanda_;
	int imgTitleSelect_;
	int imgTitleSelectBright1_;
	int imgTitleSelectBright2_;
	int imgTitleSelectBright3_;

	// フォント
	int fontHandle_;

	// 変数
	int imgPandaX_, imgPandaY_; // レッサーパンダの画像サイズ

	int textX_; // 文字の横幅

	float angleTime = 0.0f;
	float currentAngle = 0.0f;

	// 配列
	std::vector<Telop> telops_; // 複数のテロップを管理する動的配列
	
	//// スカイドーム用
	//Transform spaceDomeTran_;

	//// スカイドーム(背景)
	//SkyDome* skyDome_;

	// 惑星
	Transform planet_;
	Transform movePlanet_;

	// キャラクター
	Transform charactor_;

	// アニメーション
	AnimationController* animationController_;

};
