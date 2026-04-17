#pragma once
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Common/Grid.h"
#include "SceneBase.h"

class GameScene : public SceneBase
{

public:
	
	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

	

private:
	int pauseSelect_ = 0; // 0 = Resume, 1 = Exit

	Stage* stage_; // ステージ

	Player* player_;  // プレイヤー

	Grid grid_;  // グリッド

	Enemy* enemy_;  // 敵

	int shadowHandle_;  // 影ハンドル
	int frameCount_;  // フレームカウント
	bool showHowTo_;
	int howToImage_;

};
