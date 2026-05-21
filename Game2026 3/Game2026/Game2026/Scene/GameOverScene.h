#pragma once
#include "Scene.h"
class GameOverScene :
    public Scene
{
private:
	int gameoverStringH_;
	int backH_;
	int frame_;

	using Update_t = void(GameOverScene::*)(Input& input);
	Update_t update_;  // メンバ関数ポインタ
	using Draw_t = void(GameOverScene::*)();
	Draw_t draw_;  // メンバ関数ポインタ


	void FadeInUpdate(Input& input);

	void FadeOutUpdate(Input& input);

	// 通常更新
	void NormalUpdate(Input& input);

	void FadeDraw();

	// 通常描画
	void NormalDraw();

public:
	GameOverScene(SceneController& ctrl);
	void Update(Input& input) override;
	void Draw() override;
};

