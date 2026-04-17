#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/Camera.h"
#include "../../../Manager/SoundManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Common/AnimationController.h"
#include "../ActorBase.h"
#include "Player.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include <EffekseerForDXLib.h>
#include "../../../Common/CameraShake.h"

namespace
{
	bool CheckSphereHit(const ColliderSphere* a, const ColliderSphere* b)
	{
		if (!a || !b) return false;

		VECTOR c1 = a->GetCenter();
		VECTOR c2 = b->GetCenter();
		float  r1 = a->GetRadius();
		float  r2 = b->GetRadius();

		VECTOR d;
		d.x = c1.x - c2.x;
		d.y = c1.y - c2.y;
		d.z = c1.z - c2.z;

		float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
		float radiusSum = r1 + r2;
		float radiusSq = radiusSum * radiusSum;

		return distSq <= radiusSq;
	}
}

Player::Player(void)
{

}


Player::~Player(void)
{
}




void Player::Draw(void)
{
	
	 // モデル＋刀は共通処理を使う
	DrawModelOnly();

	// 飛ぶ刀の描画（弾）
	for (auto* p : shots)
		p->Draw();

	for (auto* f : flashes_)
		f->Draw();

	//for (auto* e : explosions_) e->Draw();

	//DrawFormatString(0, 0, 0x111111, "Effect Handle:%d", h);

	DrawFancyHpBar();

	// --- カットイン演出 ---
	if (isCutIn_)
	{
		cutInTimer_++;

		// フラッシュ（最初の10フレーム）
		if (cutInTimer_ < 10)
		{
			int alpha = 255 - cutInTimer_ * 25;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawBox(0, 0, 1920, 1080, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// カットイン画像配置（20〜60フレーム）
		if (cutInTimer_ >= 10 && cutInTimer_ < 60)
		{
			int slide = (60 - cutInTimer_) * 30; // 左から滑り込む
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
			DrawGraph(-slide, 0, cutInHandle_, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// 終了
		if (cutInTimer_ > 70)
		{
			isCutIn_ = false;
		}
	}


}

void Player::DrawModelOnly()
{
	ActorBase::Draw();

	// === 本体 ===
	SetUseBackCulling(TRUE);
	SetUsePixelShader(toonPS_);

	VECTOR cam = SceneManager::GetInstance().GetCamera()->GetPos();

	// DxLib の FLOAT4 構造体を使用
	FLOAT4 light = { 0.2f, -1.0f, 0.3f, 0.0f };
	FLOAT4 camPosF = { cam.x, cam.y, cam.z, 0.0f };
	FLOAT4 shade = { 0.7f, 0.3f, 0.0f, 0.0f };

	// ピクセルシェーダ定数設定
	SetPSConstF(0, light);
	SetPSConstF(1, camPosF);
	SetPSConstF(2, shade);

	MV1SetScale(transform_.modelId, VGet(1, 1, 1));
	MV1DrawModel(transform_.modelId);

	SetUsePixelShader(-1);

	/*if (rightHandBone_ >= 0)
	{
		MATRIX handWorld = MV1GetFrameLocalWorldMatrix(transform_.modelId, rightHandBone_);
		katana_->UpdateHandMatrix(handWorld);
		katana_->Draw();
	}*/
}

void Player::DrawShots()
{
	for (auto* p : shots)
		p->Draw();  // ここで加算ブレンド弾が描かれる
}

void Player::DrawFancyHpBar()
{
	//===============================
   // 基準位置（左上固定）
   //===============================
	const int baseX = 40;
	const int baseY = 40;

	//===============================
	// 横長 HPバー設定
	//===============================
	const int W = 320;   // 横
	const int H = 22;    // 縦

	//===============================
	// HP計算
	//===============================
	float hpRate = (float)hp_ / hpMax_;
	hpRate = max(0.0f, min(1.0f, hpRate));

	hpDelayedRate_ -= 0.01f;
	if (hpDelayedRate_ < hpRate) hpDelayedRate_ = hpRate;

	//-------------------------------
	//  ラベル「HP」表示
	//-------------------------------
	DrawString(baseX - 30, baseY + 3, "HP", GetColor(255, 255, 255));

	//-------------------------------
	//  外枠
	//-------------------------------
	DrawBox(baseX - 3, baseY - 3, baseX + W + 3, baseY + H + 3,
		GetColor(0, 0, 0), TRUE);

	DrawBox(baseX - 2, baseY - 2, baseX + W + 2, baseY + H + 2,
		GetColor(0, 255, 120), FALSE);

	//-------------------------------
	//  背景
	//-------------------------------
	DrawBox(baseX, baseY, baseX + W, baseY + H,
		GetColor(40, 40, 60), TRUE);

	//-------------------------------
	//  遅延バー（黄色）
	//-------------------------------
	int delayedRight = baseX + (int)(W * hpDelayedRate_);
	DrawBox(baseX, baseY, delayedRight, baseY + H,
		GetColor(255, 220, 80), TRUE);

	//-------------------------------
	//  現在HPバー（グラデ）
	//-------------------------------
	int hpRight = baseX + (int)(W * hpRate);

	for (int i = 0; i < H; i++)
	{
		float t = (float)i / H;
		int r = (int)(50 + 90 * t);
		int g = (int)(180 + 60 * t);
		int b = 40;

		DrawLine(baseX, baseY + i, hpRight, baseY + i,
			GetColor(r, g, b));
	}

	// ハイライト
	DrawLine(baseX, baseY, hpRight, baseY, GetColor(255, 255, 255));

	//===============================
	//  下に必殺ゲージ
	//===============================
	const int SPY = baseY + H + 24;
	const int SH = 12;

	float spRate = spGauge_ / 120.0f;
	spRate = max(0.0f, min(1.0f, spRate));

	spDelayedRate_ -= 0.02f;
	if (spDelayedRate_ < spRate) spDelayedRate_ = spRate;

	DrawString(baseX - 38, SPY, "SP", GetColor(220, 200, 255));

	DrawBox(baseX - 2, SPY - 2, baseX + W + 2, SPY + SH + 2,
		GetColor(0, 0, 0), TRUE);

	DrawBox(baseX, SPY, baseX + W, SPY + SH,
		GetColor(40, 40, 50), TRUE);

	int spDelayedRight = baseX + (int)(W * spDelayedRate_);
	DrawBox(baseX, SPY, spDelayedRight, SPY + SH,
		GetColor(180, 140, 255), TRUE);

	int spRight = baseX + (int)(W * spRate);
	DrawBox(baseX, SPY, spRight, SPY + SH,
		GetColor(120, 60, 255), TRUE);

	DrawFormatString(baseX + W + 10, SPY - 2,
		GetColor(255, 255, 255), "%d%%", (int)spGauge_);
}

void Player::Release(void)
{
	ActorBase::Release();
}

void Player::InitLoad(void)
{
	// 基底クラスのリソースロード
	//CharactorBase::InitLoad();

	transform_.SetModel(resMng_.Load(
		ResourceManager::SRC::PLAYER).handleId_);

	shot_ = resMng_.Load(ResourceManager::SRC::SHOT).handleId_;

	cutInHandle_ = LoadGraph("Data/Image/CutIn_Player.png");

	// サウンド
	SoundManager::GetInstance()->Load();

	katana_ = new Katana();
	katana_->InitLoad();
}

void Player::InitTransform(void)
{
	transform_.scl = PLAYER_SCL;
	transform_.pos = PLAYER_POS;
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);

	transform_.Update();
}

void Player::InitCollider(void)
{
	// 主に地面との衝突で仕様する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で仕様するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);

	// ★ 弾ヒット用 球コライダ（敵本体）
	bulletHitCollider_ = new ColliderSphere(
		ColliderBase::TAG::PLAYER,
		&transform_,
		VGet(0.0f, 50.0f, 0.0f),
		80.0f
	);
}

void Player::InitAnimation(void)
{
	animCtr_ = new AnimationController(transform_.modelId);
	animCtr_->Add(0, 30.0f, "Data/Model/Player/Idle.mv1");// 通常
	animCtr_->Add(1, 45.0f, "Data/Model/Player/Shiru_Walk.mv1");//  歩く
	animCtr_->Add(2, 45.0f, "Data/Model/Player/Shiru_Run.mv1");// ダッシュ
	animCtr_->Add(3, 30.0f, "Data/Model/Player/JumpRising.mv1");// ジャンプ
	animCtr_->Add(4, 30.0f, "Data/Model/Player/shiru_Attack1.mv1");// 通常攻撃１
	animCtr_->Add(5, 30.0f, "Data/Model/Player/shiru_RunAttack.mv1");// 走りながら攻撃
	animCtr_->Play(0, true);// 通常

	rightHandBone_ = MV1SearchFrame(transform_.modelId, "Hand.R");
	
}

void Player::InitPost(void)
{
	isWalk_ = false;
	isRun_ = false;
	isNAttack_ = false;
	NEffectHandle_ = -1;
	hpMax_ = 1000;
	hp_ = hpMax_;
	hpDelayedRate_ = 1.0f;

	spGauge_ = 0.0f;          // まだ必殺技溜まってない
	spDelayedRate_ = 0.0f;
}

void Player::UpdateProcess(void)
{
	// 移動操作
	ProcessMove();

	// ジャンプ処理
	//ProcessJump();
	// 通常攻撃
	NormalAttack();
	// 必殺技
	SpecialAttack();

	// 移動処理
	//transform_.pos = VAdd(transform_.pos, movePow_);

	// アニメーションごとの線分調整
	if (animCtr_->GetPlayType() == 3)
	{
		// ジャンプ中は線分を伸ばす
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::LINE)));
			colLine->SetLocalPosStart(COL_LINE_JUMP_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_JUMP_END_LOCAL_POS);
		}
	}
	else
	{
		// 通常時の線分に戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::LINE)));
			colLine->SetLocalPosStart(COL_LINE_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_END_LOCAL_POS);
		}
	}

	// 飛ぶ刀の更新 & 消去
	for (auto it = shots.begin(); it != shots.end(); )
	{
		(*it)->Update();
		if ((*it)->IsDead())
		{
			delete (*it);
			it = shots.erase(it);
		}
		else
		{
			++it;
		}
	}

	// プレイヤーの弾ー＞敵の当たり判定
	Enemy* enemy = SceneManager::GetInstance().GetEnemy();
	if (enemy)
	{
		ColliderSphere* enemyCol = enemy->GetHitCollider();

		for (auto* shot : shots)
		{
			if (!shot->IsAlive()) continue;

			ColliderSphere* shotCol = shot->GetCollider();
			if (!shotCol) continue;

			if (CheckSphereHit(shotCol, enemyCol))
			{
				int damage = 50;   // ←通常ダメージ
				if (isD_) {
					damage = 150;
					
				}
			
				enemy->Damage(damage);
				
				spGauge_ += 1.0f;
				
				enemy = SceneManager::GetInstance().GetEnemy();
				VECTOR enemyPos = enemy->GetTransform().GetPos();
				// 爆発エフェクト追加
				//explosions_.push_back(new FlashExplosion(enemyPos));
				

				shot->Kill();   // 弾を無効にする（isAlive_ = false & shotPos.clear()）
			}
		}
	}
	// ===============================
// 敵の弾 vs プレイヤーの弾
// ===============================
	Enemy* enemy2 = SceneManager::GetInstance().GetEnemy();
	if (enemy2)
	{
		auto& enemyShots = enemy2->GetShots();  // 敵の弾リスト

		for (auto* pShot : shots) // プレイヤー弾
		{
			if (!pShot->IsAlive()) continue;
			ColliderSphere* pCol = pShot->GetCollider();
			if (!pCol) continue;

			for (auto* eShot : enemyShots) // 敵弾
			{
				if (!eShot->IsAlive()) continue;
				ColliderSphere* eCol = eShot->GetCollider();
				if (!eCol) continue;

				if (CheckSphereHit(pCol, eCol))
				{
					// ★ 弾同士が衝突した！
					pShot->Kill();
					eShot->Kill();
				}
			}
		}
	}

	if (isSpecial_)
	{

		explosionDone_ = true;
		isD_ = true;
		
		// 剣を突撃モードへ
		for (auto* s : shots)
		{
			if (s->GetType() == ShotBase::TYPE::SWORD)
				s->SetFloatMode(false);
		}

			isSpecial_ = false;
	}

	// 爆発エフェクト
	if (explosionDone_ == true)
	{
		
		explosionTimer_ += SceneManager::GetInstance().GetDeltaTime();

		if (explosionTimer_ >= 1.3f)
		{
			if (!enemy) return;
			VECTOR enemyPos = enemy->GetTransform().GetPos();

			SceneManager::GetInstance().GetEffect()->Play(
				"bakuhatu",
				enemyPos
			);
			// カメラ揺れ
			StartCameraShake(16.0f, 145);
			explosionDone_ = false;
			explosionTimer_ = 0.0f;
			
		}
	
	}

	if (isD_)
	{
		float a = 0.0f;
		a += SceneManager::GetInstance().GetDeltaTime();
		if (a >= 15.0f)
		{
			isD_ = false;
			a = 0.0f;
		}
	}

	for (auto it = flashes_.begin(); it != flashes_.end(); )
	{
		(*it)->Update();
		if (!(*it)->alive)
		{
			delete (*it);
			it = flashes_.erase(it);
		}
		else
		{
			++it;
		}
	}

	// ===============================
// 敵の弾 vs プレイヤー本体
// ===============================
	Enemy* enemyForShot = SceneManager::GetInstance().GetEnemy();
	if (enemyForShot && bulletHitCollider_)
	{
		auto& enemyShots = enemyForShot->GetShots();  // 敵の弾リスト

		for (auto* eShot : enemyShots)
		{
			if (!eShot->IsAlive()) continue;

			ColliderSphere* eCol = eShot->GetCollider();
			if (!eCol) continue;

			if (CheckSphereHit(eCol, bulletHitCollider_))
			{
				// ★ 当たった！

				// 敵の弾を消す
				eShot->Kill();

				// プレイヤーにダメージ
				int damage = 50;         // 好きな値に
				hp_ -= damage;

				if (hp_ < 0) hp_ = 0;

				// HPバーの遅延用レートも更新しておくと見た目が綺麗
				float rate = (float)hp_ / hpMax_;
				if (rate < 0.0f) rate = 0.0f;
				if (hpDelayedRate_ < rate) hpDelayedRate_ = rate;

				// HP0 になったらゲームオーバーとか
				if (hp_ <= 0)
				{
					SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::OVER);
				}
			}
		}
	}
	
}

void Player::UpdateProcessPost(void)
{
}

// 移動操作
void Player::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();
	VECTOR dir = AsoUtility::VECTOR_ZERO;


	if (GetJoypadNum() == 0)
	{
		if (ins.IsNew(KEY_INPUT_W)) dir = AsoUtility::DIR_F;
		if (ins.IsNew(KEY_INPUT_A)) dir = AsoUtility::DIR_L;
		if (ins.IsNew(KEY_INPUT_S)) dir = AsoUtility::DIR_B;
		if (ins.IsNew(KEY_INPUT_D)) dir = AsoUtility::DIR_R;
	}
	else
	{
		auto padState = ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
		dir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);
	}

	if (!AsoUtility::EqualsVZero(dir))
	{
		Quaternion cameraRot = scnMng_.GetCamera()->GetQuaRotY();
		moveDir_ = Quaternion::PosAxis(cameraRot, dir);

		// 向きの更新
		float angle = atan2f(moveDir_.x, moveDir_.z);  // Y軸周りの角度（前方Z基準）
		transform_.quaRot = Quaternion::AngleAxis(angle, AsoUtility::AXIS_Y);

		// ジャンプ中はアニメーションを変えない
		if (!isJump_)
		{
			// Lシフト押している間はダッシュ
			if (CheckHitKey(KEY_INPUT_LSHIFT))
			{
				isWalk_ = false;
				isRun_ = true;
				// ダッシュ移動
				if (!isNAttack_) {
					animCtr_->Play(2, true);
				}
				movePow_ = (VScale(moveDir_, SPEED_DASH));
			
				SoundManager::GetInstance()->StopSE("walk");
				int slowFreq = (int)(44100 * 0.8f);
				SoundManager::GetInstance()->SetFrequencySE("run", slowFreq);
				SoundManager::GetInstance()->PlaySELoop("run");
				SoundManager::GetInstance()->SetVolumeSE(255, "run");
			}
			else
			{
				isRun_ = false;
				isWalk_ = true;
				// 通常移動
				animCtr_->Play(1, true);
				movePow_ = (VScale(moveDir_, SPEED_MOVE));
				SoundManager::GetInstance()->StopSE("run");
				int slowFreq = (int)(44100 * 0.5f);
				SoundManager::GetInstance()->SetFrequencySE("walk", slowFreq);
				SoundManager::GetInstance()->PlaySELoop("walk");
				SoundManager::GetInstance()->SetVolumeSE(255,"walk");

			}
		}
		
	}
	else
	{ 
		// ジャンプ中はアニメーションを変えない
		if (!isJump_)
		{
			animCtr_->Play(1, true);
			movePow_ = AsoUtility::VECTOR_ZERO;
		}
		SoundManager::GetInstance()->StopSE("walk");
		SoundManager::GetInstance()->StopSE("run");
		isRun_ = false;
		isWalk_ = false;
	}

}

void Player::ProcessJump(void)
{
	auto& ins = InputManager::GetInstance();
	bool isHitKey = ins.IsTrgDown(KEY_INPUT_SPACE)
		|| ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);
	// ジャンプ
	if (isHitKey && !isJump_)
	{
		// ジャンプ量の計算
		float jumpSpeed = POW_JUMP * scnMng_.GetDeltaTime();
		jumpPow_ = VScale(AsoUtility::DIR_U, jumpSpeed);
		isJump_ = true;
		// アニメーション再生
		animCtr_->Play(
			static_cast<int>(3), false);
	}
}

void Player::NormalAttack(void)
{
	auto& ins = InputManager::GetInstance();

	// 左クリックで攻撃開始
	if (GetMouseInput() & MOUSE_INPUT_LEFT)
	{
		
			isNAttack_ = true;
			

			// ここに実際のを書く
			if (isRun_)
			{
				animCtr_->Play(5, true); // 走り攻撃
			}
			else
			{
				
			}


			enemy_ = SceneManager::GetInstance().GetEnemy();

			// player_未取得の場合動かさない
			if (enemy_ == nullptr)
			{
				return;
			}

			// プレイヤー座標
			VECTOR enemyPos = enemy_->GetTransform().GetPos();

			float rotY = transform_.rot.y;
			SetRotationPlayingEffekseer3DEffect(NEffectHandle_, 0.0f, rotY, 0.0f);
			//// エフェクト再生（一度のみ）
			//if (NEffectHandle_ == -1)
			//{
			//	MATRIX handMat = MV1GetFrameLocalWorldMatrix(transform_.modelId, rightHandBone_);

			//	VECTOR pos = VGet(handMat.m[3][0], handMat.m[3][1], handMat.m[3][2]);
			//	SetPosPlayingEffekseer3DEffect(NEffectHandle_, pos.x, pos.y, pos.z);

			//	// 回転も右手の向きに合わせる
			//	VECTOR forward = VGet(handMat.m[2][0], handMat.m[2][1], handMat.m[2][2]);
			//	float rotY = atan2f(forward.x, forward.z);

			//	SetRotationPlayingEffekseer3DEffect(
			//		NEffectHandle_,
			//		0.0f,
			//		rotY,
			//		0.0f
			//	);
			//	// 固定化（時間を進めない）
			//	NEffectHandle_ = 
			//		SceneManager::GetInstance().GetEffect()
			//		->PlayAndStopAt("bakuhatu", transform_.pos, 1.2f);
			//}

			// エフェクト
			// h =SceneManager::GetInstance().GetEffect()->PlayFollow("bakuhatu", &transform_);
			
			float fixSecond = 1.5f;
			int fixFrame = static_cast<int>(fixSecond * 60.0f);

	
			// 弾の方向（敵→プレイヤー）
			VECTOR dir = VSub(enemyPos, transform_.pos);
			dir = VNorm(dir);

			// 敵の向きは見た目用に回転
			float targetY = atan2f(dir.x, dir.z);
			float rotateSpeed = 0.05f;
			float delta = targetY - transform_.rot.y;
			while (delta > DX_PI_F) delta -= DX_TWO_PI_F;
			while (delta < -DX_PI_F) delta += DX_TWO_PI_F;
			transform_.rot.y += delta * rotateSpeed;

			// 弾発射位置（敵の中心より少し上）
			VECTOR muzzlePos = GetRightHandWorldPos();

			for (ShotBase* shot : shots) {
				if (!shot->IsAlive()) {
					shot->CreateShot(muzzlePos, dir);
					return;  // 再利用したので終了
				}
			}

			// 弾作成
			// 弾のモデルをロード
			
			ShotBase* s = new Shot(ShotBase::TYPE::SHOT, shot_);
			s->CreateShot(muzzlePos, dir);
			shots.push_back(s);
		
	}

	// 左クリック離した瞬間の処理
	if (ins.IsRelMouseLeft())
	{
		isNAttack_ = false;
		NEffectHandle_ = -1;
	}
}

void Player::SpecialAttack()
{


	auto& ins = InputManager::GetInstance();
	if (!ins.IsTrgDown(KEY_INPUT_Q) || isSpecial_) return;
	if (spGauge_ < 100.0f) return;   // ★ 満タンじゃないと発動不可
	spGauge_ = 0.0f;                // ★ 使用してゼロに
	spDelayedRate_ = 0.0f;

	isSpecial_ = true;

	VECTOR playerPos = transform_.pos;
	int swordModel = resMng_.Load(ResourceManager::SRC::KATANA).handleId_;
	// ★ カットイン発動
	cutIn.Start();
	// カメラ揺れ
	StartCameraShake(8.0f, 20);

	const int NUM = 30;     // ← 剣の本数
	const float RADIUS = 160.0f;
	const float HEIGHT_STEP = 18.0f;
	const float SPIRAL_TURN = 1.0f; // 3 回転螺旋

	for (int i = 0; i < NUM; ++i)
	{
		float t = (float)i / NUM;
		float angle = t * DX_TWO_PI_F * SPIRAL_TURN;

		float radius = RADIUS * (1.0f - 0.4f * t); // 少しずつ狭まる円
		VECTOR pos = {
			playerPos.x + cosf(angle) * radius,
			playerPos.y + t * HEIGHT_STEP + 40.f,
			playerPos.z + sinf(angle) * radius
		};

		VECTOR dir = VNorm(VSub(playerPos, pos)); // 各剣が中心を向く

		ShotSword* s = new ShotSword(ShotBase::TYPE::SWORD, swordModel);
		s->SetFloatMode(true);
		s->CreateShot(pos, dir);
		shots.push_back(s);

		// 星屑フラッシュ風（疑似）
		flashes_.push_back(new Flash(pos));
	}

	
}


VECTOR Player::GetRightHandWorldPos()
{
	if (rightHandBone_ < 0)
	{
		return transform_.pos;
	}

	MATRIX handWorld = MV1GetFrameLocalWorldMatrix(transform_.modelId, rightHandBone_);

	VECTOR pos = VGet(
		handWorld.m[3][0],
		handWorld.m[3][1],
		handWorld.m[3][2]
	);

	return pos;
}

void Player::ResetState()
{
	// 移動状態
	isWalk_ = false;
	isRun_ = false;
	isNAttack_ = false;

	// 必殺技フラグ
	isSpecial_ = false;
	isD_ = false;
	explosionDone_ = false;
	explosionTimer_ = 0.0f;

	// HP/SP
	hp_ = hpMax_;
	hpDelayedRate_ = 1.0f;

	spGauge_ = 0.0f;
	spDelayedRate_ = 0.0f;

	// 弾の削除
	for (auto* s : shots) delete s;
	shots.clear();

	// フラッシュ削除
	for (auto* f : flashes_) delete f;
	flashes_.clear();

	// 攻撃エフェクト解除
	NEffectHandle_ = -1;

	// カットインリセット
	isCutIn_ = false;
	cutInTimer_ = 0;

	movePow_ = VGet(0, 0, 0);
}
