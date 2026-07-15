#pragma once
#include "SceneBase.h"
#include <memory>
#include "../Shader/PostEffect/PostEffect.h"
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
	void OnLoaded() override;
	void Update(void) override;
	void Draw(void) override;
	void DrawUI(void);
	void DrawPostEffect(int mainScreen);


private:

	// ステージ
	Stage* stage_;

	// スカイドーム
	SkyDome* skyDome_;

	// プレイヤー
	Player* player_;

	// 敵達
	std::unique_ptr<EnemyManager> enemyMng_;

	// ポストエフェクト
	std::unique_ptr<PostEffect> postEffect_;


	bool isHideEnemyAtStart_;
	int hideEnemyStartTime_;
	static constexpr int HIDE_ENEMY_TIME = 500; // 0.5秒

	enum class STEP
	{
		INTRO,
		PLAY,
	};

	STEP step_;

	int introTitleFont_;
	int introBodyFont_;
	int introSmallFont_;

	void DrawIntroUI(void) const;
	void DrawPlayerAndEnemy(void);
};
