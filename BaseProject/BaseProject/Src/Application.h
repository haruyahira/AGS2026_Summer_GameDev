#pragma once
#include <string>
#include <memory>
class FpsController;
// using namespace std; // これをいれると、どこからのstringかわからなくなり、
// 名前かぶりエラーがおこる可能性があります

class Application
{

public:
	/*
	* 学習メモ
	* 上のはアクセス修飾子という。
	* 関数はメンバ関数、変数はメンバ変数という
	*/

	// スクリーンサイズ
	//static constexpr int SCREEN_SIZE_X = 1024;
	//static constexpr int SCREEN_SIZE_Y = 640;

	/*
	* 学習メモ：
	* static は共通の物でこれがもしHPでするとAのHPが減った時にBのHPが減ります
	* constexpr はビルド時にはきまるので、実行中に変えることができなくなり速度も早くなります
	* int は整数　マイナス２１億～２１億まで
	*/

	// データパス関連
	//-------------------------------------------
	static const std::string PATH_IMAGE;
	static const std::string PATH_MODEL;
	static const std::string PATH_EFFECT;
	//-------------------------------------------
	/*
	* 学習メモ
	* constは「プレイヤーが入力した名前」「その時たまたま出たダメージ量」など、動か
	* してみないとわからないけど、一度決まったらその場では変えたくない値
	*/

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	/*
	* 学習メモ
	* void は処理をしたあとに数字などは返ってこない　つまり実行のみ
	*/

	// 静的インスタンスの取得
	static Application& GetInstance(void);
	/*
	* 学習メモ
	* 静的は動的に比べるとメモリの最適化、コンパイル時にエラーを教えてくれる
	* クラス＝設計図、インスタンス＝完成品
	* クラスはタイ焼きの型、インスタンスはその型に生地をいれて焼き上げたタイ焼き自体
	* Application&はなぜ＆をつけるのか　それはつけることによってコピー私ではなく
	* 本物を渡すことができます（参照渡しといいます）
	*/

	// 初期化
	void Init(void);

	// ゲームループの開始
	void Run(void);

	// リソースの破棄
	void Destroy(void);

	// 初期化成功／失敗の判定
	bool IsInitFail(void) const;

	// 解放成功／失敗の判定
	bool IsReleaseFail(void) const;

	// 解像度関連
	inline static int getSizeX_;
	inline static int getSizeY_;
	inline static int getFps_;
private:

	// 静的インスタンス
	//static Application* instance_;
	friend struct std::default_delete<Application>;
	static std::unique_ptr<Application> instance_;

	// FPS
	//FpsController* fpsController_;
    std::unique_ptr<FpsController> fpsController_;

	// 初期化失敗
	bool isInitFail_;

	// 解放失敗
	bool isReleaseFail_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	Application(void);
	Application(const Application& manager) = default;
	~Application(void) = default;

	// Effekseerの初期化
	void InitEffekseer(void);

};
