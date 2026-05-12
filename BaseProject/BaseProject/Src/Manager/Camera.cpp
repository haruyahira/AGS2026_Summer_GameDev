#include <math.h>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Object/Common/Transform.h"
#include "Camera.h"

Camera::Camera(void)
{
	angles_ = VECTOR();
	cameraUp_ = VECTOR();
	mode_ = MODE::NONE;
	pos_ = AsoUtility::VECTOR_ZERO;
	targetPos_ = AsoUtility::VECTOR_ZERO;
	followTransform_ = nullptr;
	currentGoalAngleY_ = 0.0f;
	//currentGoalAngleX_ = 0.0f;
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{

	ChangeMode(MODE::FIXED_POINT);

}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{

	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	case Camera::MODE::FIRST_PERSON:
		SetBeforeDrawFirstPerson();
		break;

	}

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		pos_, 
		targetPos_, 
		cameraUp_
	);

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::Draw(void)
{
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

void Camera::SetFirstPersonPos(const VECTOR& headPos)
{
	pos_ = headPos;

}

VECTOR Camera::GetPos(void) const
{
	return pos_;
}

VECTOR Camera::GetAngles(void) const
{
	return angles_;
}

VECTOR Camera::GetTargetPos(void) const
{
	return targetPos_;
}

Quaternion Camera::GetQuaRot(void) const
{
	return rot_;
}

Quaternion Camera::GetQuaRotOutX(void) const
{
	return rotOutX_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, pos_));
}

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		break;
	case Camera::MODE::FOLLOW:
		break;
	case Camera::MODE::FIRST_PERSON: // 一人称視点
		break;
	}

}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	pos_ = DEFAULT_CAMERA_POS;

	// 注視点
	targetPos_ = AsoUtility::VECTOR_ZERO;

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

	angles_.x = AsoUtility::Deg2RadF(30.0f);
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	rot_ = Quaternion();

}

void Camera::SyncFollow(void)
{

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// 重力の方向制御に従う
	// 正面から設定されたY軸分、回転させる
	rotOutX_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// 正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	VECTOR localPos;

	// 注視点(通常重力でいうところのY値を追従対象と同じにする)
	localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	// カメラ位置
	localPos = rot_.PosAxis(LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

}

// 一人称視点
void Camera::SyncFirstPerson(void)
{
	if (followTransform_ == nullptr) return;

	// 1. 左右回転は Follow と同じ
	rotOutX_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// 2. 【ここを修正】
	// 一人称の時だけ、上下角度 (angles_.x) を反転させてクォータニオンを作る
	// これで Follow のロジックを壊さずに一人称の視点だけを正せます
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(-angles_.x, AsoUtility::AXIS_X));

	// 3. 注視点の計算
	// Follow で注視点計算に使っている LOCAL_F2T_POS をそのまま使う
	VECTOR localPos = rot_.PosAxis(LOCAL_F2T_POS);

	// 注視点 = プレイヤーから送られる頭の位置(pos_) + 回転させたオフセット
	targetPos_ = VAdd(pos_, localPos);

	// 4. 上方向
	cameraUp_ = AsoUtility::DIR_U;
}

void Camera::ProcessRot(void)
{

	auto& ins = InputManager::GetInstance();

	float movePow = 5.0f;

	// カメラ回転
	if (ins.IsNew(KEY_INPUT_RIGHT))
	{
		// 右回転
		angles_.y += AsoUtility::Deg2RadF(1.0f);
	}
	if (ins.IsNew(KEY_INPUT_LEFT))
	{
		// 左回転
		angles_.y += AsoUtility::Deg2RadF(-1.0f);
	}

	// 上回転
	if (ins.IsNew(KEY_INPUT_UP))
	{
		angles_.x += AsoUtility::Deg2RadF(1.0f);
		if (angles_.x > LIMIT_X_UP_RAD)
		{
			angles_.x = LIMIT_X_UP_RAD;
		}
	}

	// 下回転
	if (ins.IsNew(KEY_INPUT_DOWN))
	{
		angles_.x += AsoUtility::Deg2RadF(-1.0f);
		if (angles_.x < -LIMIT_X_DW_RAD)
		{
			angles_.x = -LIMIT_X_DW_RAD;
		}
	}

	// マウスの横移動量を取得
	// --- マウス操作 ---
	int mDiffX = ins.GetMouseDiffX();
	int mDiffY = ins.GetMouseDiffY();

	// 感度の設定
	float sensitivity = 0.002f;

	// 勝手に回るのを防ぐガード (デッドゾーン)
	// InputManager側で修正済みの場合は不要ですが、ここでも念のため 0 以外を条件にします
	if (mDiffX != 0)
	{
		// 直接カメラの角度(angles_.y)を更新する
		angles_.y += (float)mDiffX * sensitivity;
	}

	if (mDiffY != 0)
	{
		// 縦方向も同様に更新
		angles_.x -= (float)mDiffY * sensitivity;
	}

	// --- 角度の制限 (リミッター) ---
	if (angles_.x > LIMIT_X_UP_RAD)  angles_.x = LIMIT_X_UP_RAD;
	if (angles_.x < -LIMIT_X_DW_RAD) angles_.x = -LIMIT_X_DW_RAD;

}

void Camera::SetBeforeDrawFixedPoint(void)
{

}

void Camera::SetBeforeDrawFollow(void)
{

	// カメラ操作
	ProcessRot();

	// 追従対象との相対位置を同期
	SyncFollow();

}

void Camera::SetBeforeDrawSelfShot(void)
{
}

void Camera::SetBeforeDrawFirstPerson(void)
{
	// カメラ操作
	ProcessRot();

	// 追従対象との相対位置を同期
	SyncFirstPerson();
}
