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
	standHeight_ = 90.0f;

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

}

void Player::Init(void)
{

	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	//MV1SetAmbColorScale(transform_.modelId, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	transform_.scl = { 0.1f, 0.1f, 0.1f };
	transform_.pos = { 0.0f, -30.0f, -50.0f };
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

	// すべてのカプセルを描画
	for (auto& pair : capsules_) {
		pair.second->Draw();
	}

	DrawFootstepDebug();

	DrawFormatString(
		20,
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


	DrawFormatString(
		20,
		45,
		GetColor(255, 100, 100),
		"HP: %d / %d",
		GetHP(),
		GetMaxHP()
	);

	auto& ins = InputManager::GetInstance();

	int rx =
		ins.GetPadAKeyRX(
			InputManager::JOYPAD_NO::PAD1);

	int ry =
		ins.GetPadAKeyRY(
			InputManager::JOYPAD_NO::PAD1);

	auto d =
		ins.GetJPadDInputState(
			InputManager::JOYPAD_NO::PAD1);

	for (int i = 0; i < 16; i++)
	{
		DrawFormatString(
			20,
			200 + i * 20,
			GetColor(255, 255, 0),
			"Button[%d] : %d",
			i,
			d.Buttons[i]
		);
	}

	
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
	flashlight_.isOn = false;
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

	// 通常・うつ伏せ切り替え (Cキー)
	// 通常・うつ伏せ切り替え (Cキー)

	bool isProneTrigger =
	ins.IsTrgDown(KEY_INPUT_C) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::R_STICK_PUSH); // 右スティック押し込み


	// ダッシュ切り替え
	if (ins.IsPadBtnTrgDown(
		InputManager::JOYPAD_NO::PAD1,
		InputManager::JOYPAD_BTN::L_STICK_PUSH))
	{
		isDash_ = !isDash_;
	}



	if (isProneTrigger && !isAttacking_)

	{
		// ⭕ 起き上がれない時は「何もしない」ように明示的に分ける
		if (IsProne())
		{
			if (isStand_)
			{
				ChangeState(STATE::PLAY); // 頭上が安全なときだけ立ち上がる
			}
			// 頭上が詰まっている（isStand_ == false）なら、Cキーを押しても無視して寝たままにする
		}
		else
		{
			ChangeState(STATE::PRONE); // 通常状態から寝るのはいつでも可能
		}
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
	const int DEAD_ZONE = 300;

	int lx = ins.GetPadAKeyLX(InputManager::JOYPAD_NO::PAD1);
	int ly = ins.GetPadAKeyLY(InputManager::JOYPAD_NO::PAD1);

	if (abs(lx) < DEAD_ZONE) lx = 0;
	if (abs(ly) < DEAD_ZONE) ly = 0;

	if (lx != 0 || ly != 0)
	{
		float stickX = static_cast<float>(lx) / 1000.0f;
		float stickY = static_cast<float>(ly) / 1000.0f;

		VECTOR padDir = AsoUtility::VECTOR_ZERO;

		// 左右
		padDir = VAdd(
			padDir,
			VScale(cameraRot.GetRight(), stickX)
		);

		// 前後
		// 多くの環境ではスティック上がマイナスなので -stickY
		padDir = VAdd(
			padDir,
			VScale(cameraRot.GetForward(), -stickY)
		);

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
			jumpPow_ = VScale(AsoUtility::DIR_U, POW_JUMP);
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

		attackTimer_ = 0.7f;


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

	// 重力と接地判定（先に地面の高さを決める）
	CollisionGravity();

	// 貫通防止ループ（壁と机の判定をセットで繰り返す）
	for (int i = 0; i < 3; i++)
	{
		CollisionCapsule(); // ステージとの判定
		CollisionBox();     // 机との判定
	}

	// 移動
	transform_.pos = movedPos_;

}

void Player::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{

		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{

			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			stepJump_ = 0.0f;


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
	if (furnitures_.empty()) return;

	isStand_ = true;

	// 身長を実際のカプセルに合わせて少し高くする（天板をすり抜けないように）
	// float pHeight = IsProne() ? 30.0f : 90.0f;
	VECTOR currentHeadPos = MV1GetFramePosition(transform_.modelId, headBoneFrame_);
	float pHeight = currentHeadPos.y - transform_.pos.y;

	for (auto f : furnitures_) {

		float pBottomY = movedPos_.y;
		float pTopY = movedPos_.y + pHeight;
		float pStandTopY = movedPos_.y + standHeight_;

		// Wallなど、独自の当たり判定を持つ家具用
		if (f->ResolveCollision(
			movedPos_,
			pRadius_,
			pBottomY,
			pStandTopY))
		{
			continue;
		}

		for (const auto& box : f->GetColliders()) {

			float pBottomY = movedPos_.y;          // 足元
			float pTopY = movedPos_.y + pHeight;// 頭
			float pStandTopY = movedPos_.y + standHeight_;
			float boxBottom = box.center.y - box.halfSize.y;
			float boxTop = box.center.y + box.halfSize.y;


			// 1. 高さのチェック（Y軸が重なっているか）
			if (pBottomY > boxTop || pStandTopY < boxBottom) continue;

			// 2. XZ平面での判定
			float minX = box.center.x - box.halfSize.x;
			float maxX = box.center.x + box.halfSize.x;
			float minZ = box.center.z - box.halfSize.z;
			float maxZ = box.center.z + box.halfSize.z;

			float closestX = fmaxf(minX, fminf(movedPos_.x, maxX));
			float closestZ = fmaxf(minZ, fminf(movedPos_.z, maxZ));

			float diffX = movedPos_.x - closestX;
			float diffZ = movedPos_.z - closestZ;
			float distSq = (diffX * diffX) + (diffZ * diffZ);

			// 半径以内なら衝突
			if (distSq < (pRadius_ * pRadius_)) {

				if (pTopY <= boxBottom && pStandTopY > boxBottom) {
					isStand_ = false; // 立ち上がりフラグを折る

					// うつ伏せ（PRONE）の時は天板の下をスムーズに通り抜けさせたいので、
					// これ以上横方向の押し出しなどの物理計算をさせずに、次の家具の判定へスキップする
					if (IsProne()) continue;
				}

				// XYZのどの方向に押し出すべきか（めり込み量が一番少ない方向）を計算する

				// 上下方向のめり込み量
				float pushUp = boxTop - pBottomY;      // 上に押し上げる量
				float pushDown = pTopY - boxBottom;      // 下に押し下げる量
				float minYPush = fminf(pushUp, pushDown);

				// 横方向のめり込み量（内部にいる場合を考慮）
				float pushLeft = (movedPos_.x - minX) + pRadius_;
				float pushRight = (maxX - movedPos_.x) + pRadius_;
				float pushFront = (movedPos_.z - minZ) + pRadius_;
				float pushBack = (maxZ - movedPos_.z) + pRadius_;

				float minXPush = fminf(pushLeft, pushRight);
				float minZPush = fminf(pushFront, pushBack);
				float minXZPush = fminf(minXPush, minZPush);

				// もし「縦（Y）のめり込み」の方が「横（XZ）のめり込み」より浅ければ、机の上に乗る（または頭をぶつける）
				if (minYPush < minXZPush) {
					if (pushUp < pushDown) {
						// 机の上に乗る
						movedPos_.y += pushUp;
						jumpPow_ = AsoUtility::VECTOR_ZERO; // ジャンプ力をリセット

						// ジャンプ入力時間のリセットと着地モーション ---
						stepJump_ = 0.0f;
						if (isJump_)
						{
							animationController_->Play(
								(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
						}
		
						isJump_ = false;
					}
					else {
						// 机の裏に頭をぶつける
						movedPos_.y -= pushDown;
						if (jumpPow_.y > 0.0f) jumpPow_.y = 0.0f; // 上昇を止める
						isStand_ = false;
					
					}
				}
				else {
					// 従来通りの横方向への押し出し
					float dist = sqrtf(distSq);
					if (dist > 0.0001f) {
						movedPos_.x = closestX + (diffX / dist) * pRadius_;
						movedPos_.z = closestZ + (diffZ / dist) * pRadius_;
					}
					else {
						// 完全に中心が一致してしまった場合の押し出し
						if (minXPush == pushLeft)       movedPos_.x -= pushLeft;
						else if (minXPush == pushRight) movedPos_.x += pushRight;
						else if (minZPush == pushFront) movedPos_.z -= pushFront;
						else                            movedPos_.z += pushBack;
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

	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 最初は実装しない。地面と突き抜けることを確認する。
	// 内積
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
		jumpPow_ = gravity;
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
	for (auto f : furnitures_)
	{
		if (f == nullptr) continue;

		if (f->IsUnder(transform_.pos))
		{
			return true;
		}
	}
	return false;
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

#ifdef _DEBUG
	//printfDx("Player Damage : %d\n", damage);
#endif

	if (HpManager::GetInstance().IsDead(this))
	{
		isDead_ = true;
		ChangeState(STATE::NONE);

		// 追加：一回でも死んだことを記録する
		SceneManager::GetInstance().SetPlayerDeadOnce(true);

#ifdef _DEBUG
		printfDx("Player Dead\n");
#endif
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