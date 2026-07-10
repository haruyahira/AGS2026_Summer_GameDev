#pragma once
#include <chrono>

class SceneBase;
class Fader;
class Camera;

class SceneManager
{

public:

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME,
		RESULT,
		GAMEOVER,
		GAMECLEAR,
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	void Init(void);
	void Init3D(void);
	void Update(void);
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


	void ResetGameResultData(void);

	void SetResultData(int stolenMoney);

	int GetGameRemainDay(void) const;

	int GetResultRemainDay(void) const;

	int GetResultStolenMoney(void) const;

	int GetResultTotalMoney(void) const;

	bool CanGoNextDay(void) const;


	int GetTargetMoney(void) const;
	int GetNeedMoney(void) const;

	void SetPlayerDeadOnce(bool isDead);
	bool IsPlayerDeadOnce(void) const;

	bool IsGameClear(void) const;

	// メインスクリーン
	int GetMainScreen(void);

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// 各種シーン
	SceneBase* scene_;

	// フェード
	Fader* fader_;

	// カメラ
	Camera* camera_;

	// シーン遷移中判定
	bool isSceneChanging_;
	// ウィンドウ判定
	bool isWindow_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);
	// コピーコンストラクタも同様
	SceneManager(const SceneManager& manager) = default;
	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	void BeginSceneLoad(void);

	// フェード
	void Fade(void);

	// メインスクリーン
	int mainScrenn_;


	static constexpr int START_REMAIN_DAY = 3;

	int gameRemainDay_;       // 現在の残り日数
	int resultRemainDay_;     // リザルト表示用
	int resultStolenMoney_;   // 今回盗んだ金額
	int resultTotalMoney_;    // 累計金額

	int targetMoney_ = 1000000;     // 目標金額
	bool isPlayerDeadOnce_ = false; // 一回でも死んだか


	bool isNowLoading_;

	void StartAsyncChangeScene(SCENE_ID sceneId);
	void DrawLoadingScreen(void);

	int loadingStartTime_;
	bool isLoadingStarted_;
	bool isLoadingDrawn_;
	static constexpr int LOADING_MIN_TIME = 1680; // ミリ秒 1000 = 1秒


};
