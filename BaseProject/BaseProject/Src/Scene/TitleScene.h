#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"
class SceneManager;
class SkyDome;
class AnimationController;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	// 画像
	int imgTitleBack_;
	int imgTitleLogo_;
	int imgTitleRedpanda_;
	int imgTitleSelect_;
	int imgTitleSelectBright1_;
	int imgTitleSelectBright2_;
	int imgTitleSelectBright3_;

	int imgW, imgH;

	// 変数
	float angleTime = 0.0f;
	float currentAngle = 0.0f;
	float driftX;

	// スカイドーム用
	Transform spaceDomeTran_;

	// スカイドーム(背景)
	SkyDome* skyDome_;

	// 惑星
	Transform planet_;
	Transform movePlanet_;

	// キャラクター
	Transform charactor_;

	// アニメーション
	AnimationController* animationController_;

};
