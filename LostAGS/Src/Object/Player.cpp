#include <string>
#include <cassert>
#include <cmath>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/Camera.h"
#include "Common/AnimationController.h"
#include "Common/Hp/HpManager.h"
#include "Collider/Capsule.h"
#include "Collider/Collider.h"
#include "Stage/Planet.h"
#include "Player.h"

Player::Player(void)
{
	animationController_ = nullptr;
	state_ = STATE::NONE;
	animType_ = ANIM_TYPE::IDLE;

	currentAnimType_ = -1;

	speed_ = 0.0f;
	moveDir_ = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;
	movedPos_ = AsoUtility::VECTOR_ZERO;

	playerRotY_ = Quaternion();
	goalQuaRot_ = Quaternion();
	stepRotTime_ = 0.0f;
	attackAngleRad_ = AsoUtility::Deg2RadF(60.0f);

	jumpPow_ = AsoUtility::VECTOR_ZERO;
	isJump_ = false;
	stepJump_ = 0.0f;
	standHeight_ = 150.0f;

	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	imgShadow_ = -1;

	capsule_ = nullptr;

	maxHp_ = 10;
	isDead_ = false;
	isDash_ = false;

	isFootstepActive_ = false;
	footstepRange_ = 0.0f;
	footstepTimer_ = 0.0f;
	footstepInterval_ = 20.0f;
	wasFallingBeforeCollision_ = false;

	isStand_ = true;
	isHiddenInTrashcan_ = false;
	isForcedProneByTrashcan_ = false;

	// 追加：ダメージエフェクト
	isDamageEffect_ = false;
	damageEffectTimer_ = 0.0f;
	damageEffectTime_ = 0.35f;
}

Player::~Player(void)
{

	HpManager::GetInstance().UnregisterHP(this);
	//delete capsule_;
	// すべてのカプセルを解放
	for (auto& pair : capsules_) {
		delete pair.second;
	}
	capsules_.clear();

	if (flashlight_.handle != -1)
	{
		DeleteLightHandle(flashlight_.handle);
		flashlight_.handle = -1;
	}

	ResourceManager::GetInstance().DeleteDuplicateModel(transform_.modelId);

}

void Player::Init(void)
{

	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	//MV1SetAmbColorScale(transform_.modelId, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	transform_.scl = { 0.1f, 0.1f, 0.1f };
	transform_.pos = { -3650.0f, -30.0f, 100.0f },
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();
	// アニメーションの設定
	InitAnimation();

	InitCollider();
	InitFlashLight();
	// 【追加】モデル全体の自己発光（エミッシブ）を完全にオフ（黒）にする
	// これにより、環境光と懐中電灯の光以外では一切光らなくなります。

	// モデル全体の「アンビエント（環境光への反応）」を標準（1.0倍）にする
	MV1SetAmbColorScale(transform_.modelId, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	// 丸影画像
	imgShadow_ = resMng_.Load(ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	// 初期状態
	ChangeState(STATE::PLAY);
	HpManager::GetInstance().RegisterHP(this, maxHp_, 60.0f);
	int head = MV1SearchFrame(transform_.modelId, "Head_Bone");
	//// 頭だけを非表示にする
	//if (head != -1) {
	//	MV1SetFrameVisible(transform_.modelId, headFrame_, FALSE);
	//}

}

void Player::Update(void)
{

	if (isDead_)
	{
		return;
	}

	// 更新ステップ
	switch (state_)
	{
	case Player::STATE::NONE:
		UpdateNone();
		break;
	case Player::STATE::PLAY:
		UpdatePlay();
		break;
	case Player::STATE::PRONE:
		UpdateProne();
		break;
	}

	// アニメーション再生
	animationController_->Update();

	// モデル制御更新
	transform_.Update();

	UpdateFlashLight();

	// ダメージエフェクト更新
	UpdateDamageEffect();

	//SetFirstPerson();
	/*capsule_->Update();*/
	// すべてのカプセルの座標更新
	for (auto& pair : capsules_) {
		pair.second->Update();
	}
	

}

void Player::Draw(void)
{

	// モデルの描画
	MV1DrawModel(transform_.modelId);

	// 丸影描画
	DrawShadow();
#ifdef _DEBUG
	{
		XINPUT_STATE x;
		ZeroMemory(&x, sizeof(x));

		int xResult = GetJoypadXInputState(DX_INPUT_PAD1, &x);

	}
#endif

#ifdef _DEBUG

	DrawFootstepDebug();

	DrawFormatString(
		100,
		20,
		GetColor(255, 255, 0),
		"Player Pos X: %.2f  Y: %.2f  Z: %.2f",
		transform_.pos.x,
		transform_.pos.y,
		transform_.pos.z
	);



	if (isAttacking_)
	{
		DrawSphere3D(
			GetAttackPos(),
			45.0f,
			16,
			GetColor(255, 255, 0),
			GetColor(255, 255, 0),
			FALSE);
	}
#endif

	
	// 画面赤フラッシュ
	DrawDamageEffect();

}

void Player::DrawUI()
{
	constexpr int HP_UI_HEIGHT = 60;
	constexpr int LEFT_MARGIN = 20;
	constexpr int BOTTOM_MARGIN = 25;

	int drawX = LEFT_MARGIN;

	int drawY =
		Application::GetScreenHeight()
		- HP_UI_HEIGHT
		- BOTTOM_MARGIN;

	hpUI_.DrawPlayerHP(
		this,
		drawX,
		drawY,
		"Player"
	);
}
void Player::AddCollider(Collider* collider)
{
	colliders_.push_back(collider);
}

void Player::ClearCollider(void)
{
	colliders_.clear();
}

const Capsule* Player::GetCapsule(void) const
{
	return capsule_;
}

const std::vector<Furniture*>& Player::GetFurnitures() const
{
	return furnitures_;
}

void Player::InitAnimation(void)
{
	std::string path = Application::PATH_MODEL + "Player/";

	animationController_ =
		std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Player.mv1", 10.0f);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + "Run.mv1", 40.0f);
	animationController_->Add((int)ANIM_TYPE::JUMP, path + "Jump.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::HIT_R, path + "Hit_R.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::HIT_L, path + "Hit_L.mv1", 30.0f);

	animationController_->Add((int)ANIM_TYPE::PRONE_IDLE, path + "ProneIdle.mv1", 0.0f);
	animationController_->Add((int)ANIM_TYPE::PRONE_WALK, path + "ProneWalk.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::PRONE_RUN, path + "ProneRun.mv1", 60.0f);

	animationController_->Add((int)ANIM_TYPE::FLY, path + "Flying.mv1", 60.0f);

	currentAnimType_ = -1;
	PlayAnimation(ANIM_TYPE::IDLE);
}
void Player::InitCollider(void)
{

	// 頭のフレーム（メッシュ）を探す
	headFrame_ = MV1SearchFrame(transform_.modelId, "Head_Bone");
	assert(headFrame_ >= 0);
	headBoneFrame_ = MV1SearchFrame(transform_.modelId, "Head_Bone_end");
	proneFrame_ = MV1SearchFrame(transform_.modelId, "Spine.001");
	SpineFrame_ = MV1SearchFrame(transform_.modelId, "Spine");

	headPos_ = MV1GetFramePosition(transform_.modelId, headFrame_);

	leftShoulderFrame_ = MV1SearchFrame(transform_.modelId, "Arm.L");
	leftHandFrame_ = MV1SearchFrame(transform_.modelId, "Hand.L");

	rightShoulderFrame_ = MV1SearchFrame(transform_.modelId, "Arm.R");
	rightHandFrame_ = MV1SearchFrame(transform_.modelId, "Hand.R");

	// カプセルコライダ
	//capsule_ = new Capsule(transform_);
	//capsule_->AttachToBone(transform_.modelId, headBoneFrame_, SpineFrame_);
	////capsule_->SetLocalPosTop(worldHeadPos_);
	////capsule_->SetLocalPosDown({ 0.0f, 15.0f, 0.0f });
	//capsule_->SetRadius(15.0f); // プレイヤーの当たり判定の大きさ

	auto* bodyCap = new Capsule(transform_);
	bodyCap->AttachToBone(transform_.modelId, headBoneFrame_, SpineFrame_);
	bodyCap->SetRadius(15.0f);
	capsules_[BONE_PART::BODY] = bodyCap;


	// 3. 左腕のカプセル（追加）
	if (leftShoulderFrame_ != -1 && leftHandFrame_ != -1) {
		auto* leftArmCap = new Capsule(transform_);
		leftArmCap->AttachToBone(transform_.modelId, leftHandFrame_, leftShoulderFrame_);
		leftArmCap->SetRadius(8.0f); // 腕なので少し細く
		capsules_[BONE_PART::LEFT_ARM] = leftArmCap;
	}

	// 4. 右腕のカプセル（追加）
	if (rightShoulderFrame_ != -1 && rightHandFrame_ != -1) {
		auto* rightArmCap = new Capsule(transform_);
		rightArmCap->AttachToBone(transform_.modelId, rightHandFrame_, rightShoulderFrame_);
		rightArmCap->SetRadius(8.0f);
		capsules_[BONE_PART::RIGHT_ARM] = rightArmCap;
	}

	pRadius_ = capsules_[BONE_PART::BODY]->GetRadius();
}

void Player::InitFlashLight(void)
{
#ifdef _DEBUG
	// テスト用
	flashlight_.range = 300000.0f;   // ← 超遠距離
	flashlight_.outerAngle = DX_PI_F / 2.0f; // ← かなり広い（90度）
	flashlight_.innerAngle = DX_PI_F / 3.0f; // ← 中心も広く

	flashlight_.handle = CreateSpotLightHandle(
		VGet(0, 0, 0),
		VGet(0, 0, 1),
		flashlight_.outerAngle,
		flashlight_.innerAngle,
		flashlight_.range,

		1.0f,   // Atten0（基本光量）
		0.0f,   // Atten1（距離減衰なし）
		0.0f    // Atten2（距離減衰なし）
	);
#else
	
	// 懐中電灯の初期設定（一直線っぽくするために数値を調整）
	flashlight_.isOn = true;
	flashlight_.range = 1500.0f;    //  少し遠くまで光を届かせる（元 1000.0f）
	flashlight_.outerAngle = 0.80f; //  光の広がりを狭くする（元 0.4f）
	flashlight_.innerAngle = 0.1f;  //  中心の強い光を狭くする（元 0.2f）
	//  定義通りの引数でスポットライトを作成する
	flashlight_.handle = CreateSpotLightHandle(
		VGet(0, 0, 0),         // Position
		VGet(0, 0, 1),         // Direction
		flashlight_.outerAngle, // OutAngle
		flashlight_.innerAngle, // InAngle
		flashlight_.range,      // Range
		1.0f,    // Atten0
		0.002f,  // Atten1
		0.000f   // Atten2
	);
#endif

	// 2. 作成したハンドルに対して、後から色を設定する
	COLOR_F color;
	color.r = 1.0f;
	color.g = 1.0f;
	color.b = 0.85f;
	color.a = 1.0f;

	SetLightDifColorHandle(flashlight_.handle, color); // ディフューズ（拡散光）の色を設定

	// 3. ライトの有効化
	SetLightEnableHandle(flashlight_.handle, flashlight_.isOn);

}

void Player::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	switch (state_)
	{
	case Player::STATE::NONE:
		ChangeStateNone();
		break;
	case Player::STATE::PLAY:
		ChangeStatePlay();
		break;
	case Player::STATE::PRONE:
		ChangeStateProne();
		break;
	}

}

void Player::ChangeStateNone(void)
{
}

void Player::ChangeStatePlay(void)
{
	if (isAttacking_)
	{
		return;
	}

	PlayAnimation(ANIM_TYPE::IDLE);
}

void Player::ChangeStateProne(void)
{
	if (isAttacking_)
	{
		return;
	}

	PlayAnimation(ANIM_TYPE::PRONE_IDLE);

}


void Player::UpdateNone(void)
{
}

void Player::UpdatePlay(void)
{
	// 共通の更新処理
	UpdateCommon();

	//攻撃処理
	ProcessAttack();
}

void Player::UpdateProne(void)
{
	// 共通の更新処理
	UpdateCommon();

	// うつ伏せ特有の処理
}

void Player::UpdateCommon(void)
{
	auto& ins = InputManager::GetInstance();

	
	// 通常・うつ伏せホールド
	bool isPronePress =
		ins.IsPress(KEY_INPUT_C) ||
		ins.IsPress(KEY_INPUT_LCONTROL) ||
		ins.IsPadBtnNew(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::R_STICK_PUSH);

	// ダッシュ切り替え
	if (ins.IsPadBtnTrgDown(
		InputManager::JOYPAD_NO::PAD1,
		InputManager::JOYPAD_BTN::L_STICK_PUSH))
	{
		isDash_ = !isDash_;
	}



	
	// 移動処理
	ProcessMove();

	// ジャンプ処理
	ProcessJump();

	// 足音範囲更新
	UpdateFootstepRange();

	// 足音再生
	UpdateFootstepSound();


	// 移動方向に応じた回転
	Rotate();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定
	Collision();


	// ---------------------------
	// ホールド式しゃがみ制御
	// ---------------------------
	if (!isAttacking_)
	{
		// ゴミ箱に隠れている、またはゴミ箱によって強制しゃがみ中なら
		// Cキーを離していても立たせない
		if (isHiddenInTrashcan_ || isForcedProneByTrashcan_)
		{
			if (!IsProne())
			{
				currentAnimType_ = -1;
				ChangeState(STATE::PRONE);
			}
		}
		else if (isPronePress)
		{
			// 押している間は常にしゃがみ
			if (!IsProne())
			{
				ChangeState(STATE::PRONE);
			}
		}
		else
		{
			// 離したら立つ（立てる場合だけ）
			if (IsProne())
			{
				if (isStand_ && CheckCanStand())
				{
					ChangeState(STATE::PLAY);
				}
			}
		}
	}

	// 回転させる
	transform_.quaRot = playerRotY_;

	// カメラをプレイヤーの頭に合わせる
	SetFirstPerson();
	
}

void Player::UpdateFlashLight(void)
{
	auto& ins = InputManager::GetInstance();

	// 押されたらライトのON/OFFを切り替える
    // ライト：右クリック or LT
	bool isLightTrigger =
		ins.IsTrgMouseRight() ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::L_TRIGGER);

	if (isLightTrigger)

	{
		flashlight_.isOn = !flashlight_.isOn;
		SetLightEnableHandle(flashlight_.handle, flashlight_.isOn);
	}

	// ライトがオフならこれ以降の座標計算はスキップ
	if (!flashlight_.isOn) return;

	// 1. ライトの発生位置（右手ボーンの世界座標）
	VECTOR lightPos = MV1GetFramePosition(transform_.modelId, rightHandFrame_);

	// 2. ライトの方向（手のボーンではなく、カメラの向いている方向にする）
	VECTOR camPos = GetCameraPosition();       // カメラの位置
	VECTOR camTarget = GetCameraTarget();      // カメラの注視点
	VECTOR lightDir = VNorm(VSub(camTarget, camPos)); // カメラの視線ベクトル（プレイヤーの正面）

	// 3. DxLibのライトハンドルに対して位置と方向を同期
	SetLightPositionHandle(flashlight_.handle, lightPos);
	SetLightDirectionHandle(flashlight_.handle, lightDir);
}

void Player::DrawShadow(void)
{
	float PLAYER_SHADOW_HEIGHT = 300.0f;
	float PLAYER_SHADOW_SIZE = 30.0f;

	int i;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3] = { VERTEX3D(), VERTEX3D(), VERTEX3D() };
	VECTOR SlideVec;
	int ModelHandle;

	// 影描画用設定
	//SetUseLighting(FALSE);
	SetUseZBuffer3D(TRUE);

	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	for (const auto c : colliders_)
	{
		if (c == nullptr)
		{
			continue;
		}

		ModelHandle = c->modelId_;

		HitResDim = MV1CollCheck_Capsule(
			ModelHandle,
			-1,
			transform_.pos,
			VAdd(transform_.pos, { 0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f }),
			PLAYER_SHADOW_SIZE);

		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);

		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		HitRes = HitResDim.Dim;

		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			SlideVec = VScale(HitRes->Normal, 0.5f);

			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		MV1CollResultPolyDimTerminate(HitResDim);
	}

	// 必ず戻す
	SetUseLighting(TRUE);
	SetUseZBuffer3D(TRUE);
}
void Player::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	movePow_ = AsoUtility::VECTOR_ZERO; 
	moveDir_ = AsoUtility::VECTOR_ZERO;

	Quaternion cameraRot =
		SceneManager::GetInstance().GetCamera()->GetQuaRotOutX();

	VECTOR dir = AsoUtility::VECTOR_ZERO;

	// -----------------------------
	// キーボード移動
	// -----------------------------
	if (ins.IsPress(KEY_INPUT_W))
	{
		dir = VAdd(dir, cameraRot.GetForward());
	}

	if (ins.IsPress(KEY_INPUT_S))
	{
		dir = VAdd(dir, cameraRot.GetBack());
	}

	if (ins.IsPress(KEY_INPUT_D))
	{
		dir = VAdd(dir, cameraRot.GetRight());
	}

	if (ins.IsPress(KEY_INPUT_A))
	{
		dir = VAdd(dir, cameraRot.GetLeft());
	}

	// -----------------------------
	// コントローラ左スティック移動
	// -----------------------------

	int lx = ins.GetPadAKeyLX(InputManager::JOYPAD_NO::PAD1);
	int ly = ins.GetPadAKeyLY(InputManager::JOYPAD_NO::PAD1);

	// コントローラーの種類を取得
	auto padType =
		ins.GetJPadType(InputManager::JOYPAD_NO::PAD1);

	// 基本は Xbox / XInput 用
	int deadZone = 8000;
	float stickMax = 32767.0f;

	// PS5 DualSense は DirectInput 扱いなので値が小さい
	if (padType == InputManager::JOYPAD_TYPE::DUAL_SENSE)
	{
		deadZone = 300;
		stickMax = 1000.0f;
	}

	// デッドゾーン
	if (abs(lx) < deadZone)
	{
		lx = 0;
	}

	if (abs(ly) < deadZone)
	{
		ly = 0;
	}

	if (lx != 0 || ly != 0)
	{
		float stickX =
			static_cast<float>(lx) / stickMax;

		float stickY =
			static_cast<float>(ly) / stickMax;

		// 念のため -1.0 ～ 1.0 に制限
		if (stickX > 1.0f) stickX = 1.0f;
		if (stickX < -1.0f) stickX = -1.0f;

		if (stickY > 1.0f) stickY = 1.0f;
		if (stickY < -1.0f) stickY = -1.0f;

		VECTOR padDir = AsoUtility::VECTOR_ZERO;

		// 左右
		padDir = VAdd(
			padDir,
			VScale(cameraRot.GetRight(), stickX)
		);

		// 前後
		if (padType == InputManager::JOYPAD_TYPE::DUAL_SENSE)
		{
			// プレステは元の挙動を維持
			padDir = VAdd(
				padDir,
				VScale(cameraRot.GetForward(), -stickY)
			);
		}
		else
		{
			// Xboxで前後が逆だったので反転
			padDir = VAdd(
				padDir,
				VScale(cameraRot.GetForward(), stickY)
			);
		}

		dir = VAdd(dir, padDir);
	}
	// 斜め移動で速くなりすぎないよう正規化
	if (!AsoUtility::EqualsVZero(dir))
	{
		dir = VNorm(dir);
	}

	bool isMove = !AsoUtility::EqualsVZero(dir);


	bool isRun =
		ins.IsPress(KEY_INPUT_LSHIFT) ||
		ins.IsPress(KEY_INPUT_RSHIFT) ||
		isDash_;


	bool canMove = (isJump_ || IsEndLanding());

	if (isMove && canMove)
	{
		float currentSpeed = 0.0f;

		if (IsProne())
		{
			currentSpeed = isRun ? 200.0f : 100.0f;
		}
		else
		{
			currentSpeed = isRun ? SPEED_RUN : SPEED_MOVE;
		}

		moveDir_ = dir;
		movePow_ = VScale(
			dir,
			currentSpeed * scnMng_.GetDeltaTime());


		if (!isAttacking_)
		{
			if (IsProne())
			{
				if (isRun)
				{
					PlayAnimation(ANIM_TYPE::PRONE_RUN);
				}
				else
				{
					PlayAnimation(ANIM_TYPE::PRONE_WALK);
				}
			}
			else
			{
				if (isRun)
				{
					PlayAnimation(ANIM_TYPE::FAST_RUN);
				}
				else
				{
					PlayAnimation(ANIM_TYPE::RUN);
				}
			}
		}
	}
	else
	{
		if (!isAttacking_)
		{
			if (!isJump_ && IsEndLanding())
			{
				if (IsProne())
				{
					PlayAnimation(ANIM_TYPE::PRONE_IDLE);
				}
				else
				{
					PlayAnimation(ANIM_TYPE::IDLE);
				}
			}
		}
	}
}

void Player::ProcessJump(void)
{
	InputManager& ins = InputManager::GetInstance();

	// 押した瞬間だけ true
	bool isJumpTrigger =
	ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN); // Xbox:A / PS:×


	// 押している間 true
	bool isJumpPress =
	ins.IsNew(KEY_INPUT_SPACE) ||
		ins.IsPadBtnNew(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);




	// ジャンプ開始
	   // Spaceを押した瞬間、かつジャンプ中ではない、かつ着地している時だけ
	if (isJumpTrigger && !isJump_ && IsEndLanding())
	{
		animType_ = ANIM_TYPE::JUMP;
		currentAnimType_ = static_cast<int>(ANIM_TYPE::JUMP);

		animationController_->Play(
			static_cast<int>(ANIM_TYPE::JUMP),
			true,
			13.0f,
			25.0f);

		animationController_->SetEndLoop(
			23.0f,
			25.0f,
			5.0f);

		isJump_ = true;
		stepJump_ = 0.0f;
	}

	// ジャンプ中の上昇処理
	// 押しっぱなしなら一定時間だけ上昇力を与える
	if (isJump_ && isJumpPress)
	{
		stepJump_ += scnMng_.GetDeltaTime();

		if (stepJump_ < TIME_JUMP_IN)
		{
			// 0.0f ～ 1.0f に正規化
			float t = stepJump_ / TIME_JUMP_IN;

			// 念のため範囲制限
			if (t < 0.0f) t = 0.0f;
			if (t > 1.0f) t = 1.0f;

			// 最初は強く、後半は弱くなる
			float ease = AsoUtility::EaseOutJump(t);

			jumpPow_ = VScale(AsoUtility::DIR_U, POW_JUMP * ease);
		}
	}

	// Spaceを離したら、それ以上ジャンプを伸ばさない
	if (!isJumpPress)
	{
		stepJump_ = TIME_JUMP_IN;
	}

}

void Player::ProcessAttack(void)
{
	auto& ins = InputManager::GetInstance();

	// 攻撃開始

	bool isAttackTrigger =
		ins.IsTrgMouseLeft() ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::R_TRIGGER);


	if (!isAttacking_ && isAttackTrigger)
	{
		isAttacking_ = true;


		auto& snd = SoundManager::GetInstance();

		snd.SetSEVolume(255);
		snd.PlaySE(SoundManager::SE::ATTACK);

		attackTimer_ = 0.6f;


		if (isRightAttack_)
		{
			animType_ = ANIM_TYPE::HIT_R;
			currentAnimType_ = static_cast<int>(ANIM_TYPE::HIT_R);

			animationController_->Play(
				static_cast<int>(ANIM_TYPE::HIT_R),
				false);
		}
		else
		{
			animType_ = ANIM_TYPE::HIT_L;
			currentAnimType_ = static_cast<int>(ANIM_TYPE::HIT_L);

			animationController_->Play(
				static_cast<int>(ANIM_TYPE::HIT_L),
				false);
		}

		isRightAttack_ = !isRightAttack_;
	}

	// 攻撃中
	// 攻撃中
	if (isAttacking_)
	{
		attackTimer_ -= scnMng_.GetDeltaTime();

		if (attackTimer_ <= 0.0f)
		{
			isAttacking_ = false;

			if (IsProne())
			{
				currentAnimType_ = -1;
				PlayAnimation(ANIM_TYPE::PRONE_IDLE);
			}
			else
			{
				currentAnimType_ = -1;
				PlayAnimation(ANIM_TYPE::IDLE);
			}
		}
	}
}
void Player::SetGoalRotate(double rotRad)
{

	VECTOR cameraRot = SceneManager::GetInstance().GetCamera()->GetAngles();
	Quaternion axis = Quaternion::AngleAxis((double)cameraRot.y + rotRad, AsoUtility::AXIS_Y);

	// 現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;

}

void Player::Rotate(void)
{
	//playerRotY_ = Quaternion::Slerp(playerRotY_, goalQuaRot_, 0.2f);
	
	//stepRotTime_ -= scnMng_.GetDeltaTime();

	// カメラの現在の水平角度（Y軸）を取得
	float cameraAngleY = SceneManager::GetInstance().GetCamera()->GetAngles().y;

	// キャラが常にカメラと同じ方向を向くように目標角度を設定
	goalQuaRot_ = Quaternion::AngleAxis(cameraAngleY, AsoUtility::AXIS_Y);

	// 0.2fの部分を大きくすると、カメラの回転にキャラがより素早く追従します
	// 1.0f にすると完全に遊びがなく同期します
	playerRotY_ = Quaternion::Slerp(playerRotY_, goalQuaRot_, 0.2f);
	//// 回転の球面補間
	//playerRotY_ = Quaternion::Slerp(
	//	playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

}

void Player::Collision(void)
{
	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// CollisionGravityでjumpPow_が0になる前に、落下中か記録しておく
	wasFallingBeforeCollision_ =
		VDot(AsoUtility::DIR_D, jumpPow_) > 0.001f;

	// 重力と接地判定
	CollisionGravity();

	// 貫通防止ループ
	for (int i = 0; i < 3; i++)
	{
		CollisionCapsule();
		CollisionBox();
	}

	transform_.pos = movedPos_;
}

void Player::CollisionGravity(void)
{
	// FPS補正
	float dtScale = scnMng_.GetDeltaTime() * FPS_BASE;

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力移動前の位置
	VECTOR beforeGravityPos = movedPos_;

	// 重力移動量
	VECTOR gravityMove = VScale(jumpPow_, dtScale);

	// 重力移動後の位置
	VECTOR afterGravityPos = VAdd(beforeGravityPos, gravityMove);

	// とりあえず移動後位置を入れる
	movedPos_ = afterGravityPos;

	// 落下中かどうか
	bool isFalling = VDot(dirGravity, jumpPow_) > 0.0f;

	// 落下中でなければ、床着地判定はしない
	if (!isFalling)
	{
		return;
	}

	// -----------------------------------------
	// 地面チェック用の線
	// 落下前より少し上から、落下後より十分下まで見る
	// -----------------------------------------
	const float CHECK_UP = 80.0f;
	const float CHECK_DOWN = 120.0f;

	VECTOR lineStart = VAdd(beforeGravityPos, VScale(dirUpGravity, CHECK_UP));
	VECTOR lineEnd = VAdd(afterGravityPos, VScale(dirGravity, CHECK_DOWN));

	bool isHitGround = false;
	VECTOR nearestHitPos = AsoUtility::VECTOR_ZERO;

	float nearestY = -999999.0f;

	for (const auto c : colliders_)
	{
		if (c == nullptr)
		{
			continue;
		}

		auto hit = MV1CollCheck_Line(
			c->modelId_,
			-1,
			lineStart,
			lineEnd);

		if (hit.HitFlag > 0)
		{
			// 一番高い床を採用する
			if (!isHitGround || hit.HitPosition.y > nearestY)
			{
				isHitGround = true;
				nearestY = hit.HitPosition.y;
				nearestHitPos = hit.HitPosition;
			}
		}
	}

	if (isHitGround)
	{
		// 地面の少し上に座標を補正
		movedPos_ = VAdd(nearestHitPos, VScale(dirUpGravity, 2.0f));

		// 落下速度リセット
		jumpPow_ = AsoUtility::VECTOR_ZERO;
		stepJump_ = 0.0f;

		// 着地アニメーション
		if (isJump_)
		{
			animType_ = ANIM_TYPE::JUMP;
			currentAnimType_ = static_cast<int>(ANIM_TYPE::JUMP);

			animationController_->Play(
				static_cast<int>(ANIM_TYPE::JUMP),
				false,
				29.0f,
				45.0f,
				false,
				true);
		}

		isJump_ = false;
	}
}

void Player::CollisionCapsule(void)
{

	// すべてのボーンカプセルに対してステージ（壁や障害物）との衝突を計算
	for (auto& pair : capsules_) {
		Capsule* cap = pair.second;
		if (cap == nullptr) continue; // 安全対策

		// 現在の計算中の一時的な移動後座標を適用した仮想カプセルを作成
		Transform trans = Transform(transform_);
		trans.pos = movedPos_;
		trans.Update();

		// 元のカプセルのボーン相対位置を維持したまま、予測移動先にずらす
		Capsule tempCap = Capsule(*cap, trans);

		for (const auto c : colliders_) {
			// ステージモデルとのカプセル総当たりチェック
			auto hits = MV1CollCheck_Capsule(
				c->modelId_, -1,
				tempCap.GetPosTop(), tempCap.GetPosDown(), tempCap.GetRadius());

			for (int i = 0; i < hits.HitNum; i++) {
				auto hit = hits.Dim[i];

				// 腕や頭の判定が「床（真上を向いているポリゴン）」に反応して
				// プレイヤーが浮き上がるのを防ぐガード句。
				// 法線(Normal)が真上を向いている（傾きが緩やか＝床）なら、横方向の壁判定からは除外します。
				if (hit.Normal.y > 0.7f) {
					continue;
				}

				for (int tryCnt = 0; tryCnt < 10; tryCnt++) {
					// 修正：capDownPos ではなく tempCap.GetPosDown() を直接使います
					int pHit = HitCheck_Capsule_Triangle(
						tempCap.GetPosTop(), tempCap.GetPosDown(), tempCap.GetRadius(),
						hit.Position[0], hit.Position[1], hit.Position[2]);

					if (pHit) {
						// 壁から押し出す
						movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
						trans.pos = movedPos_;
						trans.Update();

						// カプセルの位置も同期して更新
						new (&tempCap) Capsule(*cap, trans);
						continue;
					}
					break;
				}
			}
			// 検出した地面ポリゴン情報の後始末
			MV1CollResultPolyDimTerminate(hits);
		}
	}

}

void Player::CollisionBox()
{

	// 毎フレーム、基本は立てる状態に戻す
	isStand_ = true;


	if (furnitures_.empty())
	{
		return;
	}

	isStand_ = true;

	// 身長を実際のカプセルに合わせる
	VECTOR currentHeadPos =
		MV1GetFramePosition(
			transform_.modelId,
			headBoneFrame_);

	float pHeight =
		currentHeadPos.y - transform_.pos.y;

	// -----------------------------------------------------
	// 前フレームの足元・頭の高さ
	// 横から近づいただけで机に乗らないように使う
	// -----------------------------------------------------
	float prevBottomY = transform_.pos.y;
	float prevTopY = transform_.pos.y + pHeight;

	for (auto f : furnitures_)
	{
		if (f == nullptr)
		{
			continue;
		}

		float pBottomY = movedPos_.y;
		float pTopY = movedPos_.y + pHeight;
		float pStandTopY = movedPos_.y + standHeight_;

		// -----------------------------------------------------
		// Wallなど、独自の当たり判定を持つ家具用
		// -----------------------------------------------------

		if(f->GetOBBColliders().empty())
		{
			if (f->ResolveCollision(
				movedPos_,
				pRadius_,
				pBottomY,
				pStandTopY))
			{
				continue;
			}
		}


		// -----------------------------------------------------
// OBB Collider 判定
// Tableなど、回転・スケールする家具用
// -----------------------------------------------------
		bool isOBBHit = false;

		for (const auto& obb : f->GetOBBColliders())
		{
			pBottomY = movedPos_.y;
			pTopY = movedPos_.y + pHeight;
			pStandTopY = movedPos_.y + standHeight_;

			float obbTopY = obb.GetTopY();
			float obbBottomY = obb.GetBottomY();

			const float TOP_MARGIN = 10.0f;
			const float BOTTOM_MARGIN = 10.0f;
			const float UNDER_MARGIN = 10.0f;

			// -------------------------------------------------
			// 0. 机の下に潜っている判定
			// -------------------------------------------------

			// 移動後の頭位置を作る
			VECTOR currentHeadPos =
				MV1GetFramePosition(transform_.modelId, headBoneFrame_);

			VECTOR headLocal =
				VSub(currentHeadPos, transform_.pos);

			VECTOR movedHeadPos =
				VAdd(movedPos_, headLocal);

			VECTOR movedCenterPos =
				VScale(VAdd(movedPos_, movedHeadPos), 0.5f);

			// この家具の下にいるか
			bool isUnderThisFurniture =
				f->IsUnder(movedPos_) ||
				f->IsUnder(movedHeadPos) ||
				f->IsUnder(movedCenterPos);

			if (IsProne() &&
				isUnderThisFurniture &&
				pTopY <= obbBottomY - UNDER_MARGIN)
			{
				// 立ったら頭が机にぶつかるなら立てない
				if (pStandTopY > obbBottomY - 10.0f)
				{
					isStand_ = false;
				}

				continue;
			}


		
			// -------------------------------------------------
            // 1. 机の上に乗る判定
            // -------------------------------------------------

           // CollisionGravityでjumpPow_が0になる前の落下状態を見る
			bool isFallingDown = wasFallingBeforeCollision_;

			// 足が少し天板にめり込んでも拾えるようにする
			const float LAND_TOP_MARGIN = 30.0f;
			const float LAND_DEPTH = 80.0f;

			// 前フレームでは天板より上付近、今フレームでは天板付近〜少し下
			bool isTableTopHeight =
				prevBottomY >= obbTopY - 5.0f &&
				pBottomY <= obbTopY + LAND_TOP_MARGIN &&
				pBottomY >= obbTopY - LAND_DEPTH;

			// XZ判定
			// IsUnderは机の下判定なので、Yだけ机の下に仮置きしてXZ判定として使う
			VECTOR checkXZPos = movedPos_;

			// ResolveCollisionXZ は座標を書き換えるので、必ずコピーで判定する
			bool isOnTableXZ =
				obb.ResolveCollisionXZ(
					checkXZPos,
					pRadius_,
					obbBottomY - 1000.0f,
					obbTopY + 1000.0f
				);



			if (isFallingDown && isTableTopHeight && isOnTableXZ)
			{
				// 机の上に乗せる
				movedPos_.y = obbTopY + 2.0f;

				jumpPow_ = AsoUtility::VECTOR_ZERO;
				stepJump_ = 0.0f;

				if (isJump_)
				{
					animationController_->Play(
						static_cast<int>(ANIM_TYPE::JUMP),
						false,
						29.0f,
						45.0f,
						false,
						true);
				}

				isJump_ = false;

				isOBBHit = true;
				break;
			}

			// -------------------------------------------------
			// 2. 机の裏に頭をぶつける判定
			// -------------------------------------------------
			bool isHitBottom =
				prevTopY <= obbBottomY + BOTTOM_MARGIN &&
				pTopY >= obbBottomY - BOTTOM_MARGIN &&
				jumpPow_.y > 0.0f;

			if (isHitBottom)
			{
				if (obb.ResolveCollisionBottom(
					movedPos_,
					pRadius_,
					pBottomY,
					pTopY))
				{
					jumpPow_.y = 0.0f;
					isStand_ = false;

					isOBBHit = true;
					break;
				}
			}

			// -------------------------------------------------
			// 3. 横方向の押し出し
			// ここが重要：
			// pStandTopY ではなく、実際の頭の高さ pTopY を使う
			// -------------------------------------------------
			if (obb.ResolveCollisionXZ(
				movedPos_,
				pRadius_,
				pBottomY,
				pTopY))
			{
				isOBBHit = true;
				break;
			}
		}

		// OBBで解決した家具は、BoxCollider判定へ進まない
		if (isOBBHit)
		{
			continue;
		}

		// -----------------------------------------------------
		// 既存 BoxCollider 判定
		// Wall / Showcase / Ceiling など用
		// -----------------------------------------------------
		for (const auto& box : f->GetColliders())
		{
			pBottomY = movedPos_.y;
			pTopY = movedPos_.y + pHeight;
			pStandTopY = movedPos_.y + standHeight_;

			float boxBottom = box.center.y - box.halfSize.y;
			float boxTop = box.center.y + box.halfSize.y;

			// 高さが重なっていないならスキップ
			if (pBottomY > boxTop || pStandTopY < boxBottom)
			{
				continue;
			}

			// XZ平面での最近点
			float minX = box.center.x - box.halfSize.x;
			float maxX = box.center.x + box.halfSize.x;
			float minZ = box.center.z - box.halfSize.z;
			float maxZ = box.center.z + box.halfSize.z;

			float closestX =
				fmaxf(
					minX,
					fminf(movedPos_.x, maxX));

			float closestZ =
				fmaxf(
					minZ,
					fminf(movedPos_.z, maxZ));

			float diffX = movedPos_.x - closestX;
			float diffZ = movedPos_.z - closestZ;

			float distSq =
				(diffX * diffX) +
				(diffZ * diffZ);

			// 半径以内なら衝突
			if (distSq < pRadius_ * pRadius_)
			{
				// 立ち上がり判定
				if (pTopY <= boxBottom && pStandTopY > boxBottom)
				{
					isStand_ = false;

					if (IsProne())
					{
						continue;
					}
				}

				float pushUp = boxTop - pBottomY;
				float pushDown = pTopY - boxBottom;
				float minYPush = fminf(pushUp, pushDown);

				float pushLeft = (movedPos_.x - minX) + pRadius_;
				float pushRight = (maxX - movedPos_.x) + pRadius_;
				float pushFront = (movedPos_.z - minZ) + pRadius_;
				float pushBack = (maxZ - movedPos_.z) + pRadius_;

				float minXPush = fminf(pushLeft, pushRight);
				float minZPush = fminf(pushFront, pushBack);
				float minXZPush = fminf(minXPush, minZPush);

				if (minYPush < minXZPush)
				{
					if (pushUp < pushDown)
					{
						// 上に乗る
						movedPos_.y += pushUp;

						jumpPow_ = AsoUtility::VECTOR_ZERO;
						stepJump_ = 0.0f;

						if (isJump_)
						{
							animationController_->Play(
								static_cast<int>(ANIM_TYPE::JUMP),
								false,
								29.0f,
								45.0f,
								false,
								true);
						}

						isJump_ = false;
					}
					else
					{
						// 下から頭をぶつける
						movedPos_.y -= pushDown;

						if (jumpPow_.y > 0.0f)
						{
							jumpPow_.y = 0.0f;
						}

						isStand_ = false;
					}
				}
				else
				{
					// 横方向の押し出し
					float dist = sqrtf(distSq);

					if (dist > 0.0001f)
					{
						movedPos_.x =
							closestX +
							(diffX / dist) * pRadius_;

						movedPos_.z =
							closestZ +
							(diffZ / dist) * pRadius_;
					}
					else
					{
						if (minXPush == pushLeft)
						{
							movedPos_.x -= pushLeft;
						}
						else if (minXPush == pushRight)
						{
							movedPos_.x += pushRight;
						}
						else if (minZPush == pushFront)
						{
							movedPos_.z -= pushFront;
						}
						else
						{
							movedPos_.z += pushBack;
						}
					}
				}
			}
		}
	}

}


void Player::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	// FPS補正
	float dtScale = scnMng_.GetDeltaTime() * FPS_BASE;

	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow * dtScale);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 内積
	float dot = VDot(dirGravity, jumpPow_);

	if (dot >= 0.0f)
	{
		// 落下中なら最低限の落下速度にする
		jumpPow_ = VScale(dirGravity, gravityPow);
	}
}
bool Player::IsEndLanding(void)
{

	bool ret = true;

	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}

	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;
	}

	return false;

}

void Player::SetFirstPerson(void)
{
	// カメラのポインタを取得
	auto camera = SceneManager::GetInstance().GetCamera();

	// カメラ視点関連
	// 頭のボーン位置を取得（transform_.Update() の後に行うこと）
	//headFrame_ = MV1SearchFrame(transform_.modelId, "Head_Bone");
	headPos_ = MV1GetFramePosition(transform_.modelId, headBoneFrame_);

	//worldHeadPos_ = MV1GetFramePosition(transform_.modelId, headBoneFrame_);
	//localHeadPos_ = VSub(worldHeadPos_, transform_.pos);

	//// カプセルの頭位置を頭のボーン位置に合わせる
	//capsule_->SetLocalPosTop(localHeadPos_);


	// カメラの座標を頭の位置に上書き設定する関数を呼ぶ
	// (Cameraクラスに SetPos 関数などを追加して pos_ = headPos する)
	// うつ伏せなら、Y座標をちょっと下げる（これだけ！）
	if (IsProne())
	{
		headPos_.y -= 11.0f;
	}


	// カメラ位置
	VECTOR cameraPos = headPos_;

	// カメラを小さい球として扱う
	float cameraRadius = 5.0f;

	// 家具・壁・天井とのカメラ衝突
	for (auto f : furnitures_)
	{
		f->ResolveCameraCollision(
			cameraPos,
			cameraRadius);
	}

	camera->SetFirstPersonPos(cameraPos);

}

bool Player::IsHiddenUnderFurniture() const
{

	// ゴミ箱の中に入って閉じている時
		if (isHiddenInTrashcan_)
		{
			return true;
		}

	// うつ伏せ中だけ隠れる
	if (!IsProne())
	{
		return false;
	}

	// 立てない場所なら隠れている扱い
	return !const_cast<Player*>(this)->CheckCanStand();
}

VECTOR Player::GetAttackPos() const
{

	// 右パンチ中
	if (animType_ == ANIM_TYPE::HIT_R)
	{
		return MV1GetFramePosition(
			transform_.modelId,
			rightHandFrame_);
	}

	// 左パンチ中
	if (animType_ == ANIM_TYPE::HIT_L)
	{
		return MV1GetFramePosition(
			transform_.modelId,
			leftHandFrame_);
	}

	// 攻撃していない時はプレイヤー位置
	return transform_.pos;

}

void Player::PlayAnimation(ANIM_TYPE animType, bool isLoop)
{
	if (animationController_ == nullptr)
	{
		return;
	}

	int nextAnimType = static_cast<int>(animType);

	// 同じアニメーションなら再生し直さない
	if (currentAnimType_ == nextAnimType)
	{
		return;
	}

	this->animType_ = animType;
	currentAnimType_ = nextAnimType;

	animationController_->Play(nextAnimType, isLoop);
}

bool Player::Damage(int damage)
{
	bool isDamaged =
		HpManager::GetInstance().Damage(this, damage);

	if (!isDamaged)
	{
		return false;
	}

	// ダメージエフェクト開始
	StartDamageEffect();

	if (HpManager::GetInstance().IsDead(this))
	{
		isDead_ = true;
		ChangeState(STATE::NONE);

		SceneManager::GetInstance().SetPlayerDeadOnce(true);
	}

	return true;
}
bool Player::IsDead(void) const
{
	return isDead_;
}

int Player::GetHP(void) const
{
	const Hp* hp = HpManager::GetInstance().GetHP(this);

	if (hp == nullptr)
	{
		return 0;
	}

	return hp->GetCurrent();
}

int Player::GetMaxHP(void) const
{
	const Hp* hp = HpManager::GetInstance().GetHP(this);

	if (hp == nullptr)
	{
		return 0;
	}

	return hp->GetMax();
}


VECTOR Player::GetPos(void) const
{
	return transform_.pos;
}

void Player::SetPos(const VECTOR& pos)
{
	transform_.pos = pos;
	transform_.Update();
}

VECTOR Player::GetForward(void) const
{
	auto camera = SceneManager::GetInstance().GetCamera();

	VECTOR camPos = GetCameraPosition();
	VECTOR camTarget = GetCameraTarget();

	VECTOR forward = VSub(camTarget, camPos);
	forward.y = 0.0f;

	if (VSize(forward) < 0.001f)
	{
		return VGet(0.0f, 0.0f, 1.0f);
	}

	return VNorm(forward);
}
bool Player::IsFlashLightOn() const
{
	return flashlight_.isOn;
}

VECTOR Player::GetFlashLightPos() const
{
	if (rightHandFrame_ != -1)
	{
		return MV1GetFramePosition(
			transform_.modelId,
			rightHandFrame_
		);
	}

	VECTOR pos = transform_.pos;
	pos.y += 60.0f;

	return pos;
}

VECTOR Player::GetFlashLightDir() const
{
	VECTOR camPos =
		GetCameraPosition();

	VECTOR camTarget =
		GetCameraTarget();

	VECTOR dir =
		VSub(camTarget, camPos);

	if (VSize(dir) < 0.001f)
	{
		return VGet(0.0f, 0.0f, 1.0f);
	}

	return VNorm(dir);
}

void Player::UpdateFootstepRange()
{
	auto& ins = InputManager::GetInstance();

	// 初期化
	isFootstepActive_ = false;
	footstepRange_ = 0.0f;

	// 移動しているか
	bool isMove =
		!AsoUtility::EqualsVZero(moveDir_);


	// ジャンプ中は足音を出さない
	if (!isMove || isJump_)
	{
		return;
	}

	// 着地していない時も足音なし
	if (!IsEndLanding())
	{
		return;
	}

	isFootstepActive_ = true;

	// うつ伏せ中は小さい足音
	if (IsProne())
	{
		footstepRange_ = FOOTSTEP_RANGE_PRONE;
		return;
	}

	// Shift中は大きい足音


	if (ins.IsPress(KEY_INPUT_LSHIFT) ||
		ins.IsPress(KEY_INPUT_RSHIFT) ||
		isDash_)


	{
		footstepRange_ = FOOTSTEP_RANGE_RUN;
	}
	else
	{
		footstepRange_ = FOOTSTEP_RANGE_WALK;
	}
}

#ifdef _DEBUG
void Player::DrawFootstepDebug() const
{
	int color = GetColor(0, 180, 255);

	DrawFormatString(
		20,
		70,
		color,
		"Footstep Active : %s  Range : %.1f",
		isFootstepActive_ ? "TRUE" : "FALSE",
		footstepRange_
	);

	if (!isFootstepActive_)
	{
		return;
	}

	if (footstepRange_ <= 0.0f)
	{
		return;
	}

	VECTOR center = transform_.pos;

	// 地面と重なって見えなくなるのを防ぐため、少し上に描画
	center.y += 1.0f;

	// 足音範囲を円で表示
	const int DIV = 64;
	const float PI = 3.1415926535f;

	for (int i = 0; i < DIV; i++)
	{
		float angle1 = 2.0f * PI * i / DIV;
		float angle2 = 2.0f * PI * (i + 1) / DIV;

		VECTOR p1 = VGet(
			center.x + sinf(angle1) * footstepRange_,
			center.y,
			center.z + cosf(angle1) * footstepRange_
		);

		VECTOR p2 = VGet(
			center.x + sinf(angle2) * footstepRange_,
			center.y,
			center.z + cosf(angle2) * footstepRange_
		);

		DrawLine3D(p1, p2, color);
	}
}
#endif

bool Player::IsFootstepActive() const
{
	return isFootstepActive_;
}

float Player::GetFootstepRange() const
{
	return footstepRange_;
}

VECTOR Player::GetFootstepPos() const
{
	return transform_.pos;
}

void Player::UpdateFootstepSound()
{
	if (!isFootstepActive_)
	{

		footstepTimer_ = 0.0f;

		auto& snd = SoundManager::GetInstance();

		snd.StopSE(SoundManager::SE::WALK);
		snd.StopSE(SoundManager::SE::RUN);

		return;

	}

	footstepTimer_ += scnMng_.GetDeltaTime();

	auto& ins = InputManager::GetInstance();


	bool isRun =
		ins.IsPress(KEY_INPUT_LSHIFT) ||
		ins.IsPress(KEY_INPUT_RSHIFT) ||
		isDash_;


	// 歩きと走りで間隔変更
	footstepInterval_ = isRun ? 0.3f : 0.6f;

	if (footstepTimer_ < footstepInterval_)
	{
		return;
	}

	footstepTimer_ = 0.0f;

	auto& snd = SoundManager::GetInstance();

	if (isRun)
	{
		snd.SetSEVolume(255);

		snd.PlaySE(SoundManager::SE::RUN);
	}
	else
	{
		snd.SetSEVolume(255);

		snd.PlaySE(SoundManager::SE::WALK);
	}
}
bool Player::CheckCanStand()
{
	if (furnitures_.empty())
	{
		return true;
	}

	const float SAFE_MARGIN = 15.0f;

	VECTOR footPos = transform_.pos;

	float pBottomY = transform_.pos.y;
	float pTopY = transform_.pos.y + standHeight_;

	float standTopY = pTopY;

	for (auto f : furnitures_)
	{
		if (f == nullptr)
		{
			continue;
		}

		// -----------------------------------------
		// OBB Collider 判定
		// -----------------------------------------
		for (const auto& obb : f->GetOBBColliders())
		{
			float obbBottomY = obb.GetBottomY();

			// 足元より低いものは机の脚などとして無視
			if (obbBottomY <= transform_.pos.y + 30.0f)
			{
				continue;
			}

			// ResolveCollisionXZ は位置を書き換えるので、
			// 判定用のコピーを使う
			VECTOR testPos = footPos;

			bool isXZHit = obb.ResolveCollisionXZ(
				testPos,
				pRadius_,
				pBottomY,
				pTopY
			);

			// XZ的に重なっていて、立った頭が天板下面を超えるなら立てない
			if (isXZHit && standTopY > obbBottomY - SAFE_MARGIN)
			{
				return false;
			}
		}

		// -----------------------------------------
		// Box Collider 判定
		// -----------------------------------------
		for (const auto& box : f->GetColliders())
		{
			float boxBottomY = box.center.y - box.halfSize.y;

			// 足元より低いものは無視
			if (boxBottomY <= transform_.pos.y + 30.0f)
			{
				continue;
			}

			float minX = box.center.x - box.halfSize.x;
			float maxX = box.center.x + box.halfSize.x;
			float minZ = box.center.z - box.halfSize.z;
			float maxZ = box.center.z + box.halfSize.z;

			float closestX = fmaxf(minX, fminf(footPos.x, maxX));
			float closestZ = fmaxf(minZ, fminf(footPos.z, maxZ));

			float diffX = footPos.x - closestX;
			float diffZ = footPos.z - closestZ;

			float distSq = diffX * diffX + diffZ * diffZ;

			bool isXZHit = distSq <= pRadius_ * pRadius_;

			if (isXZHit && standTopY > boxBottomY - SAFE_MARGIN)
			{
				return false;
			}
		}
	}

	return true;
}

bool Player::IsUnderFurnitureXZ() const
{
	VECTOR footPos = transform_.pos;
	VECTOR headPos = MV1GetFramePosition(transform_.modelId, headBoneFrame_);

	VECTOR centerPos = VScale(VAdd(footPos, headPos), 0.5f);

	for (auto f : furnitures_)
	{
		if (f == nullptr)
		{
			continue;
		}

		// 足元・頭・中間のどれかが机の下なら「下にいる」とする
		if (f->IsUnder(footPos) ||
			f->IsUnder(headPos) ||
			f->IsUnder(centerPos))
		{
			return true;
		}
	}

	return false;
}

void Player::SetHiddenInTrashcan(bool isHidden)
{
	isHiddenInTrashcan_ = isHidden;
}

bool Player::IsHiddenInTrashcan(void) const
{
	return isHiddenInTrashcan_;
}

void Player::ApplyTrashcanHideState(void)
{
	// =========================
	// ゴミ箱に隠れている時
	// =========================
	if (isHiddenInTrashcan_)
	{
		// ゴミ箱によって強制しゃがみにした記録
		isForcedProneByTrashcan_ = true;

		// 攻撃中ならキャンセルしてもOK
		if (isAttacking_)
		{
			isAttacking_ = false;
			attackTimer_ = 0.0f;
		}

		// 立っているなら強制しゃがみ
		if (!IsProne())
		{
			currentAnimType_ = -1;
			ChangeState(STATE::PRONE);
		}

		// 足音停止
		isFootstepActive_ = false;
		footstepRange_ = 0.0f;
		footstepTimer_ = 0.0f;

		auto& snd = SoundManager::GetInstance();
		snd.StopSE(SoundManager::SE::WALK);
		snd.StopSE(SoundManager::SE::RUN);

		return;
	}

	// =========================
	// ゴミ箱から出た時
	// =========================
	if (isForcedProneByTrashcan_)
	{
		isForcedProneByTrashcan_ = false;

		// ゴミ箱によってしゃがまされていたなら解除
		if (IsProne())
		{
			if (CheckCanStand())
			{
				currentAnimType_ = -1;
				ChangeState(STATE::PLAY);
			}
		}
	}
}

void Player::StartDamageEffect()
{
	isDamageEffect_ = true;
	damageEffectTimer_ = damageEffectTime_;

	// プレイヤーモデルを赤くする
	MV1SetDifColorScale(
		transform_.modelId,
		GetColorF(1.0f, 0.25f, 0.25f, 1.0f)
	);
}

void Player::UpdateDamageEffect()
{
	if (!isDamageEffect_)
	{
		return;
	}

	damageEffectTimer_ -= scnMng_.GetDeltaTime();

	if (damageEffectTimer_ <= 0.0f)
	{
		damageEffectTimer_ = 0.0f;
		isDamageEffect_ = false;

		// 色を元に戻す
		MV1SetDifColorScale(
			transform_.modelId,
			GetColorF(1.0f, 1.0f, 1.0f, 1.0f)
		);
	}
}


void Player::DrawDamageEffect()
{
	if (!isDamageEffect_)
	{
		return;
	}

	// 残り時間の割合
	float rate = damageEffectTimer_ / damageEffectTime_;

	if (rate < 0.0f) rate = 0.0f;
	if (rate > 1.0f) rate = 1.0f;

	// 透明度
	int alpha = static_cast<int>(180.0f * rate);

	// 画面サイズ
	int screenW = Application::SCREEN_SIZE_X;
	int screenH = Application::adjustedSizeY_;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	DrawBox(
		0,
		0,
		screenW,
		screenH,
		GetColor(255, 0, 0),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}