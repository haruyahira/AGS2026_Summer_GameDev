#pragma once
#include "Scene.h"
#include"../Geometry.h"
class GameScene :
    public Scene
{

private:
	Position2 pos_;
	Position2 vel_;
	Position2 bulletPos_;
	Position2 bulletVel_;

	bool deadBullet_;
	bool isJumping_ = false;
    int gameH_;
	int frame_;

    using Update_t = void(GameScene::*)(Input& input);
	Update_t update_;  // メンバ関数ポインタ
    using Draw_t = void(GameScene::*)();
    Draw_t draw_;  // メンバ関数ポインタ
    

	void FadeInUpdate(Input& input);

	void FadeOutUpdate(Input& input);

	// 通常更新
	void NormalUpdate(Input& input);

	void FadeDraw();
	
	// 通常描画
	void NormalDraw();


    public:
        GameScene(SceneController& ctrl);
    void Update(Input& input) override;
	void Draw() override;
};

