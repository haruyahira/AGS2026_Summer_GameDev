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
	switch (mode_)
	{
	case MODE::FIXED_POINT:
		// 固定カメラなので更新なし
		break;

	case MODE::FOLLOW:
		// マウス、右スティックによる回転
		ProcessRot();

		// 追従対象との位置同期
		SyncFollow();
		break;

	case MODE::FIRST_PERSON:
		// マウス、右スティックによる回転
		ProcessRot();

		// 一人称カメラの位置と注視点を同期
		SyncFirstPerson();
		break;

	default:
		break;
	}
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
    auto& ins =
        InputManager::GetInstance();

    // =============================
    // マウス視点移動
    // =============================

    const int mouseDiffX =
        ins.GetMouseDiffX();

    const int mouseDiffY =
        ins.GetMouseDiffY();

    const float mouseSensitivity =
        0.002f;

    if (mouseDiffX != 0)
    {
        angles_.y +=
            static_cast<float>(mouseDiffX) *
            mouseSensitivity;
    }

    if (mouseDiffY != 0)
    {
        angles_.x -=
            static_cast<float>(mouseDiffY) *
            mouseSensitivity;
    }

    // =============================
    // 右スティックを取得
    // =============================

    int rightX =
        ins.GetPadAKeyRX(
            InputManager::JOYPAD_NO::PAD1
        );

    int rightY =
        ins.GetPadAKeyRY(
            InputManager::JOYPAD_NO::PAD1
        );

    const bool isXInput =
        ins.IsXInputPad(
            InputManager::JOYPAD_NO::PAD1
        );

    // =============================
    // DualSenseなどのDirectInput
    // =============================

    if (!isXInput)
    {
        // GetJoypadAnalogInputRight()の値は
        // 基本的に-1000から1000
        const int deadZone = 100;

        if (abs(rightX) < deadZone)
        {
            rightX = 0;
        }

        if (abs(rightY) < deadZone)
        {
            rightY = 0;
        }

        float stickX =
            static_cast<float>(rightX) /
            1000.0f;

        float stickY =
            static_cast<float>(rightY) /
            1000.0f;

        // -1.0から1.0へ制限
        if (stickX > 1.0f)
        {
            stickX = 1.0f;
        }
        else if (stickX < -1.0f)
        {
            stickX = -1.0f;
        }

        if (stickY > 1.0f)
        {
            stickY = 1.0f;
        }
        else if (stickY < -1.0f)
        {
            stickY = -1.0f;
        }

        const float dualSenseSensitivity =
            0.04f;

        // 左右
        angles_.y +=
            stickX *
            dualSenseSensitivity;

        // 上がマイナス、下がプラスなので反転
        angles_.x -=
            stickY *
            dualSenseSensitivity;
    }
    // =============================
    // XboxのXInput
    // =============================
    else
    {
        const int deadZone = 8000;

        if (abs(rightX) < deadZone)
        {
            rightX = 0;
        }

        if (abs(rightY) < deadZone)
        {
            rightY = 0;
        }

        float stickX =
            static_cast<float>(rightX) /
            32767.0f;

        float stickY =
            static_cast<float>(rightY) /
            32767.0f;

        // -1.0から1.0へ制限
        if (stickX > 1.0f)
        {
            stickX = 1.0f;
        }
        else if (stickX < -1.0f)
        {
            stickX = -1.0f;
        }

        if (stickY > 1.0f)
        {
            stickY = 1.0f;
        }
        else if (stickY < -1.0f)
        {
            stickY = -1.0f;
        }

        const float xboxSensitivity =
            0.04f;

        // Xbox側は既存の向きを維持
        angles_.y +=
            stickX *
            xboxSensitivity;

        angles_.x +=
            stickY *
            xboxSensitivity;
    }

    // =============================
    // 上下角度制限
    // =============================

    if (angles_.x >
        LIMIT_X_UP_RAD)
    {
        angles_.x =
            LIMIT_X_UP_RAD;
    }

    if (angles_.x <
        -LIMIT_X_DW_RAD)
    {
        angles_.x =
            -LIMIT_X_DW_RAD;
    }
}

void Camera::SetBeforeDrawFixedPoint(void)
{

}

void Camera::SetBeforeDrawFollow(void)
{

	//// カメラ操作
	//ProcessRot();

	//// 追従対象との相対位置を同期
	//SyncFollow();

}

void Camera::SetBeforeDrawSelfShot(void)
{
}

void Camera::SetBeforeDrawFirstPerson(void)
{
	//// カメラ操作
	//ProcessRot();

	//// 追従対象との相対位置を同期
	//SyncFirstPerson();
}
