#pragma once
#include <chrono>
#include <DxLib.h>
#include "Camera.h"
#include "EffectManager.h"
//#include "../Object/Actor/Charactor/Player.h"
class SceneBase;
class Fader;
class Player;
class Enemy;
class EffectManager;

class SceneManager
{

public:

	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 0;
	static constexpr int BACKGROUND_COLOR_G = 139;
	static constexpr int BACKGROUND_COLOR_B = 139;

	// ディレクショナルライトの方向
	static constexpr VECTOR LIGHT_DIRECTION = { 0.3f, -0.7f, 0.8f };

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME,
		CLEAR,
		OVER
		
	};
	
	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	// 初期化
	void Init(void);
	
	// 3Dの初期化
	void Init3D(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;

	// カメラの取得
	Camera* GetCamera(void) const;

	// プレイヤーの取得
	Player* GetPlayer(void) const;

	// 敵の取得
	Enemy* GetEnemy(void) const;

	// エフェクトの取得
	EffectManager* GetEffect(void) const;

	bool IsPause() const { return isPause_; }
	void SetPause(bool f) { isPause_ = f; }

	bool isEnd_ = false;

	bool IsEnd() const { return isEnd_; }

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// フェード
	Fader* fader_;

	// 各種シーン
	SceneBase* scene_;

	// カメラ
	Camera* camera_;

	// プレイヤー
	Player* player_;

	// 敵
	Enemy* enemy_;

	// エフェクト
	EffectManager* effect_;

	// シーン遷移中判定
	bool isSceneChanging_;
	bool isPause_ = false;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;
	
	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);

	// コピーコンストラクタも同様
	SceneManager(const SceneManager& instance) = default;

	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード
	void Fade(void);

	int pauseSelect_ = 0;

	
	void RequestEnd() { isEnd_ = true; }

};