#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"
class SceneManager;


class TitleScene : public SceneBase
{

public:

	// テロップ1つ分のデータ
	struct  Telop  {
		int x;
	    int y;          // 座標
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
	static constexpr float RESET_FADE = 0.0f;      // フェードの速さをリセットする値
	static constexpr float RAISE＿FADE = 0.035f;      // フェードの速さを上げる値
	static constexpr float ALPHA_HALF_RANGE = 0.5f; // サイン波の振幅調整用
	static constexpr float ALPHA_OFFSET = 0.5f;     // サイン波の底上げ用
	static constexpr int   MAX_ALPHA = 255;          // 最大不透明度

	void InitSelect(void);

	// 描画
	void DrawRedpanda(void);
	void DrawSelect(void);
	void DrawTelop(void);

	// 更新
	void UpdateRedpanda(void);
	void UpdateSelect(void);
	void UpdateTelop(void);

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
	int imgTitleSelect2_;
	/*int imgTitleSelectBright1_;
	int imgTitleSelectBright2_;
	int imgTitleSelectBright3_;*/
	int imgSelectHandles_[4];
	int drawIndex_ = 0; // 現在どの画像を表示するか (0:何もなし, 1~3:光る)


	// 当たり判定用の矩形構造体
	struct Rect { int x, y, w, h; };
	Rect btnRects_[3];


	// フォント
	int fontHandle_;

	// 変数
	int imgPandaX_;
	int imgPandaY_; // レッサーパンダの画像サイズ

	int textX_; // 文字の横幅

	float angleTime_;
	float currentAngle_;
	float fadeCount_;

	// 配列
	std::vector<Telop> telops_; // 複数のテロップを管理する動的配列
	

};
