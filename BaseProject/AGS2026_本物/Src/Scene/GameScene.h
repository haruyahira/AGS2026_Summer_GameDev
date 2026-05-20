#pragma once
#include "SceneBase.h"
#include <memory>
class Stage;
class SkyDome;
class Player;
class EnemyManager;

class GameScene : public SceneBase
{

public:
	
	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	// ステージ
	Stage* stage_;

	// スカイドーム
	SkyDome* skyDome_;

	// プレイヤー
	Player* player_;

	// 敵達
	std::unique_ptr<EnemyManager> enemyMng_;
	
};
