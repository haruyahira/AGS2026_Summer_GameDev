#include <string>
#include <cassert>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Camera.h"
#include "Common/AnimationController.h"
#include "Collider/Capsule.h"
#include "Collider/Collider.h"
#include "Planet.h"
#include "Player.h"

Player::Player(void)
{

	animationController_ = nullptr;
	state_ = STATE::NONE;
	animType_ = ANIM_TYPE::IDLE;

	speed_ = 0.0f;
	moveDir_ = AsoUtility::VECTOR_ZERO;
	movePow_ = AsoUtility::VECTOR_ZERO;
	movedPos_ = AsoUtility::VECTOR_ZERO;

	playerRotY_ = Quaternion();
	goalQuaRot_ = Quaternion();
	stepRotTime_ = 0.0f;

	jumpPow_ = AsoUtility::VECTOR_ZERO;
	isJump_ = false;
	stepJump_ = 0.0f;
	standHeight_ = 90.0f;
	

	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	imgShadow_ = -1;

	capsule_ = nullptr;

}

Player::~Player(void)
{
	//delete capsule_;
	// すべてのカプセルを解放
	for (auto& pair : capsules_) {
		delete pair.second;
	}
	capsules_.clear();
}

void Player::Init(void)
{

	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	//MV1SetAmbColorScale(transform_.modelId, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	transform_.scl = { 0.1f, 0.1f, 0.1f };
	transform_.pos = { 0.0f, -30.0f, 0.0f };
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
	int head = MV1SearchFrame(transform_.modelId, "Head_Bone");
	//// 頭だけを非表示にする
	//if (head != -1) {
	//	MV1SetFrameVisible(transform_.modelId, headFrame_, FALSE);
	//}

}

void Player::Update(void)
{

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

	//capsule_->Draw();
	// すべてのカプセルを描画
	for (auto& pair : capsules_) {
		pair.second->Draw();
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

void Player::InitAnimation(void)
{

	std::string path = Application::PATH_MODEL + "Player/";
	animationController_ = std::make_unique <AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Player.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + "FastRun.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::JUMP, path + "Jump.mv1", 60.0f);

	animationController_->Add((int)ANIM_TYPE::PRONE_IDLE, path + "ProneIdle.mv1", 0.0f);
	animationController_->Add((int)ANIM_TYPE::PRONE_WALK, path + "ProneWalk.mv1", 30.0f);
	animationController_->Add((int)ANIM_TYPE::PRONE_RUN, path + "ProneRun.mv1", 60.0f);


	animationController_->Add((int)ANIM_TYPE::WARP_PAUSE, path + "WarpPose.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::FLY, path + "Flying.mv1", 60.0f);
	
	animationController_->Play((int)ANIM_TYPE::IDLE);

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

	// 懐中電灯の初期設定（一直線っぽくするために数値を調整）
	flashlight_.isOn = false;
	flashlight_.range = 1500.0f;    // 【変更】少し遠くまで光を届かせる（元 1000.0f）
	flashlight_.outerAngle = 0.80f; // 【変更】光の広がりを狭くする（元 0.4f）
	flashlight_.innerAngle = 0.1f;  // 【変更】中心の強い光を狭くする（元 0.2f）

	// 1. 定義通りの引数でスポットライトを作成する
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
}

void Player::ChangeStateProne(void)
{
}

void Player::UpdateNone(void)
{
}

void Player::UpdatePlay(void)
{
	// 共通の更新処理
	UpdateCommon();
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
	if (ins.IsTrgDown(KEY_INPUT_C))
	{
		// ⭕【修正】起き上がれない時は「何もしない」ように明示的に分ける
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

	// Fキーが押されたらライトのON/OFFを切り替える
	if (ins.IsTrgDown(KEY_INPUT_F))
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

	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// 影を落とすモデルの数だけ繰り返し
	for (const auto c : colliders_)
	{

		// チェックするモデルは、jが0の時はステージモデル、1以上の場合はコリジョンモデル
		ModelHandle = c->modelId_;

		// プレイヤーの直下に存在する地面のポリゴンを取得
		HitResDim = MV1CollCheck_Capsule(
			ModelHandle, -1,
			transform_.pos, VAdd(transform_.pos, { 0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f }), PLAYER_SHADOW_SIZE);

		// 頂点データで変化が無い部分をセット
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		// 球の直下に存在するポリゴンの数だけ繰り返し
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			// ポリゴンの座標は地面ポリゴンの座標
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			// ちょっと持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			// ポリゴンの不透明度を設定する
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = static_cast<int>(roundf(128.0f * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT)));

			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = static_cast<int>(roundf(128.0f * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT)));

			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = static_cast<int>(roundf(128.0f * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT)));

			// ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

			// 影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);
	}

	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);

}

void Player::ProcessMove(void)
{

	auto& ins = InputManager::GetInstance();

	// 移動量をゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	// X軸回転を除いた、重力方向に垂直なカメラ角度(XZ平面)を取得
	Quaternion cameraRot = SceneManager::GetInstance().GetCamera()->GetQuaRotOutX();

	// 回転したい角度
	double rotRad = 0;

	VECTOR dir = AsoUtility::VECTOR_ZERO;


	// カメラ方向に前進したい
	if (ins.IsPress(KEY_INPUT_W))
	{
		rotRad = AsoUtility::Deg2RadD(0.0);
		dir = cameraRot.GetForward();
	}

	// カメラ方向から後退したい
	if (ins.IsPress(KEY_INPUT_S))
	{
		rotRad = AsoUtility::Deg2RadD(180.0);
		dir = cameraRot.GetBack();
	}

	// カメラ方向から右側へ移動したい
	if (ins.IsPress(KEY_INPUT_D))
	{
		rotRad = AsoUtility::Deg2RadD(90.0);
		dir = cameraRot.GetRight();
	}

	// カメラ方向から左側へ移動したい
	if (ins.IsPress(KEY_INPUT_A))
	{
		rotRad = AsoUtility::Deg2RadD(270.0);
		dir = cameraRot.GetLeft();
	}



	if (!AsoUtility::EqualsVZero(dir) && (isJump_ || IsEndLanding())) {

		
		// 状態を判定
		bool isRun = ins.IsPress(KEY_INPUT_LSHIFT);

		// 速度：しゃがみなら1.2f、立ちならShift判定あり
		float currentSpeed = IsProne() ?
			(isRun ? 7.5f : 2.5f)       // しゃがみ（高速 / 通常）
			:    
			(isRun ? SPEED_RUN : SPEED_MOVE);      // 立ち（ダッシュ / 歩き）

		// 移動処理
		moveDir_ = dir;
		movePow_ = VScale(dir, currentSpeed);

		
		// 回転処理
		//SetGoalRotate(rotRad);

		// アニメショーン変更処理
		 animType_ = IsProne() ?
			(isRun ? ANIM_TYPE::PRONE_RUN : ANIM_TYPE::PRONE_WALK) // しゃがみ
			:
			(isRun ? ANIM_TYPE::FAST_RUN : ANIM_TYPE::RUN); // 立ち
		 
	   
		
	}
	else // 移動入力がない場合
	{
		if (!isJump_ && IsEndLanding())
		{
			animType_ = IsProne() ?
				ANIM_TYPE::PRONE_IDLE
				:
				ANIM_TYPE::IDLE;

		}
	}

	// アニメーション変更実行
	animationController_->Play((int)animType_);

}

void Player::ProcessJump(void)
{

	bool isHit = CheckHitKey(KEY_INPUT_SPACE);

	// ジャンプ
	if (isHit && (isJump_ || IsEndLanding()))
	{

		if (!isJump_)
		{
			// 制御無しジャンプ
			//mAnimationController->Play((int)ANIM_TYPE::JUMP);
			// ループしないジャンプ
			//mAnimationController->Play((int)ANIM_TYPE::JUMP, false);
			// 切り取りアニメーション
			//mAnimationController->Play((int)ANIM_TYPE::JUMP, false, 13.0f, 24.0f);
			// 無理やりアニメーション
			animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
			animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);
		}

		isJump_ = true;

		// ジャンプの入力受付時間をヘラス
		stepJump_ += scnMng_.GetDeltaTime();
		if (stepJump_ < TIME_JUMP_IN)
		{
			jumpPow_ = VScale(AsoUtility::DIR_U, POW_JUMP);
		}


	}
	// ボタンを離したらジャンプ力に加算しない
	if (!isHit)
	{
		stepJump_ = TIME_JUMP_IN;
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
				// 着地モーション
				animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
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

	// 最後に、決定した位置をカメラに送る（通常・うつ伏せ共通）
	camera->SetFirstPersonPos(headPos_);
}
