#pragma once
#include <DxLib.h>
#include "../Application.h"
#include "../Common/Quaternion.h"
class Transform;

class Camera
{

public:

	// カメラの初期座標
	static constexpr VECTOR DERFAULT_POS = { 0.0f, 200.0f, 500.0f };
	
	// カメラの初期角度
	static constexpr VECTOR DERFAULT_ANGLES = { 0.0f, 0.0f, 0.0f};

	// カメラの回転量
	const float ROT_POW_DEG = 2.0f;
	const float ROT_POW_RAD = ROT_POW_DEG * DX_PI_F / 180.0f;

	// カメラの移動スピード
	static constexpr float SPEED = 20.0f;

	// カメラのクリップ範囲
	static constexpr float VIEW_NEAR = 1.0f;
	static constexpr float VIEW_FAR = 30000.0f;

	// 追従位置からカメラ位置までの相対座標
	static constexpr VECTOR FOLLOW_CAMERA_LOCAL_POS = { 0.0f, 200.0f, -450.0f };

	// 追従位置から注視点までの相対座標
	static constexpr VECTOR FOLLOW_TARGET_LOCAL_POS = { 0.0f, 0.0f, 350.0f };

	// 追従対象からの相対座標
	static constexpr VECTOR FOLLOW_LOCAL_POS = { 0.0f, 0.0f, 500.0f };

	// カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 40.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 10.0f * (DX_PI_F / 180.0f);

	// マウスの中心座標
	static constexpr int centerX_ = Application::SCREEN_SIZE_HALF_X;
	static constexpr int centerY_ = Application::SCREEN_SIZE_HALF_Y;
	
	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		FREE,
		FOLLOW,
	};

	// コンストラクタ
	Camera(void);

	// デストラクタ
	~Camera(void);

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// 描画前のカメラ設定
	void SetBeforeDraw(void);

	// デバッグ用描画
	void DrawDebug(void);

	// 解放
	void Release(void);

	// 座標の取得
	const VECTOR& GetPos(void) const;

	// 角度の取得
	const VECTOR& GetAngles(void) const;
	const Quaternion& GetQuaRot(void) const;
	
	// X回転を抜いたY軸のみのカメラ角度
	const Quaternion& GetQuaRotY(void) const;
	
	// 注視点の取得
	const VECTOR& GetTargetPos(void) const;

	// カメラの前方方向
	VECTOR GetForward(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);

	// 追従対象の設定
	void SetFollow(const Transform* follow);

	void SetShake(VECTOR shake);

private:

	// カメラが追従対象とするTransform
	const Transform* followTransform_;

	// カメラモード
	MODE mode_;

	// カメラの位置
	VECTOR pos_;

	// カメラ角度(rad)
	VECTOR angles_;

	// カメラ角度(Y軸のみ)
	Quaternion rotY_;

	// カメラ角度
	Quaternion rot_;

	// 1フレーム前のマウス座標
	int mouseOX_;
	int mouseOY_;

	// マウス座標
	int mouseX_;
	int mouseY_;

	VECTOR shakeOffset_ = VGet(0, 0, 0);

	// 注視点
	VECTOR targetPos_;

	// カメラの上方向
	VECTOR cameraUp_;
	
	// カメラを初期位置に戻す
	void SetDefault(void);

	// 追従対象との位置同期を取る
	void SyncFollow(void);

	// カメラ操作
	void ProcessRot(bool isLimit);
	void ProcessMove(void);

	// カメラ回転(キーボード)
	void RotKeyboard(bool isLimit);

	// カメラ回転(ゲームパッド)
	void RotGamePad(bool isLimit);

	// モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFree(void);
	void SetBeforeDrawFollow(void);

};
