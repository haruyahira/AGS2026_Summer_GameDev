#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../../Scene/GameScene.h"
#include "../../Common/AnimationController.h"
#include "../../Collider/ColliderLine.h"
#include "../Shot/ShotBase.h"
#include "../Shot/ShotStraight.h"
#include "Player.h"
#include "Enemy.h"

Enemy::Enemy()
{

}

// モデルロード
void Enemy::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(
		ResourceManager::SRC::ENEMY).handleId_);

	shot_ = resMng_.Load(ResourceManager::SRC::ROCK).handleId_;
}

void Enemy::InitTransform(void)
{
	transform_.scl = { 1.0f, 1.0f, 1.0f };
	transform_.pos = { 200.0f, 0.0f, 150.0f };

	// モデルの角度
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);

	transform_.Update();
}

void Enemy::InitCollider(void)
{
	// 主に地面との衝突で仕様する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// ★ 弾ヒット用 球コライダ（敵本体）
	collider_ = new ColliderSphere(
		ColliderBase::TAG::ENEMY,
		&transform_,
		VGet(0.0f, 50.0f, 0.0f),
		80.0f
	);
}

void Enemy::InitAnimation(void)
{
	animCtr_ = new AnimationController(transform_.modelId);
}

void Enemy::InitPost(void)
{
	isNotice_ = false;
	isHear_ = false;
	isAlive_ = true;
	// 初期向き設定
	//dirEnemy = VTransform(AsoUtility::DIR_F, MGetRotY(angles_.y));
	cntAttack_ = TERM_ATTACK;
	isAttack_ = false;
	fireInterval_ = 60.0f;
	fireTimer_ = 0.0f;
	hp_ = 300000;
	maxHp_ = 300000;
	damageFlashTimer_ = 0.0f;
	damageFont_ = CreateFontToHandle(
		"ＭＳ ゴシック",
		32,
		5,
		DX_FONTTYPE_EDGE
	);

}

void Enemy::Update(void)
{
	ActorBase::Update();

	// ダメージ点滅
	if (damageFlashTimer_ > 0.0f)
	{
		damageFlashTimer_ -= SceneManager::GetInstance().GetDeltaTime();
	}

	// 攻撃カウントしていく
	cntAttack_ += TERM_ATTACK * SceneManager::GetInstance().GetDeltaTime();
	if (cntAttack_ >= TERM_ATTACK)
	{
		isAttack_ = true;
	}

	// 弾の更新
	for (ShotBase* shots : shots_) {
		shots->Update();
	}
	cnt += 0.05f;
	if (cnt >= 1.0f)
	{
		FireToPlayer();
		cnt = 0.0f;
	}

	Coliision();

	// ----- ダメージポップアップ更新 -----
	for (auto& p : damagePopups_)
	{
		p.pos.y += p.vy;                      // 上昇
		p.pos.x += sinf((1.0f - p.timer) * 10.0f) * p.vxOffset; // 横揺れ

		p.timer -= SceneManager::GetInstance().GetDeltaTime();
		p.alpha = p.timer;                    // 時間でフェードアウト
	}

}

void Enemy::Draw(void)
{
	ActorBase::Draw();
	DrawModelOnly();

	for (ShotBase* shots : shots_) {
		shots->Draw();
	}

	// 体力ゲージ
	DrawFancyHpBar();

	// ----- ダメージポップアップ描画 -----
	for (auto& p : damagePopups_)
	{
		// ワールド → 画面座標（HPバーと同じやり方）
		VECTOR sp = ConvWorldPosToScreenPos(p.pos);

		int x = (int)sp.x;
		int y = (int)sp.y - 40;   // HPバーより少し上にずらす

		// デバッグ用：位置確認
		DrawPixel(x, y, GetColor(255, 0, 0));

		// 色（ダメージ量で少し変化させる）
		int r = 255;
		int g = max(0, (int)(200 - p.value * 0.4f));
		int b = 40;
		int col = GetColor(r, g, b);

		// 透明度
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(p.alpha * 255));

		// 影（黒・少しずらす）
		DrawFormatString(
			x + 2,
			y + 2,
			GetColor(0, 0, 0),
			"%d",
			(int)p.value
		);

		// 本体
		DrawFormatString(
			x,
			y,
			col,
			"%d",
			(int)p.value
		);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// ===== ダメージポップアップの削除 =====
	damagePopups_.erase(
		std::remove_if(
			damagePopups_.begin(),
			damagePopups_.end(),
			[](const DamagePopup& p) {
				return p.timer <= 0.0f;
			}
		),
		damagePopups_.end()
	);


	if (collider_)
	{
		collider_->DrawDebug(0xff0000);  // 赤色で描画
	}
}

void Enemy::DrawModelOnly()
{
	int matCount = MV1GetMaterialNum(transform_.modelId);

	if (damageFlashTimer_ > 0.0f)
	{
		// ★ すべてのマテリアルを光らせる
		for (int i = 0; i < matCount; i++)
		{
			MV1SetMaterialDifColor(transform_.modelId, i, GetColorF(1.8f, 1.3f, 1.3f, 1.0f));
			MV1SetMaterialEmiColor(transform_.modelId, i, GetColorF(0.6f, 0.5f, 0.5f, 1.0f));
		}
	}

	MV1DrawModel(transform_.modelId);

	// ★ 元に戻す
	if (damageFlashTimer_ > 0.0f)
	{
		for (int i = 0; i < matCount; i++)
		{
			MV1SetMaterialDifColor(transform_.modelId, i, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
			MV1SetMaterialEmiColor(transform_.modelId, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
		}
	}
}

void Enemy::Release(void)
{
	ActorBase::Release();

	// 弾の解放
	for (ShotBase* shot : shots_) {
		shot->Release();
		delete shot;
	}
}



void Enemy::ProcessMove(void)
{
	
}

void Enemy::DrawFancyHpBar()
{
	// ■ 位置計算（今のコードを流用）
	VECTOR pos = transform_.pos;
	pos.y += 520.0f;
	pos.z += 50.0f;

	VECTOR sp = ConvWorldPosToScreenPos(pos);
	int x = (int)sp.x;
	int y = (int)sp.y;

	const int W = 180;      // ★ 幅：太め
	const int H = 18;       // ★ 高さ：大きめ
	int left = x - W / 2;
	int top = y - H / 2;

	// ■ HP比
	float rate = (float)hp_ / maxHp_;
	if (rate < 0) rate = 0;
	if (rate > 1) rate = 1;

	// ■ Apex風ダメージ遅延バー
	static float delayedRate = 1.0f;
	delayedRate -= 0.01f;                     // 徐々に減る
	if (delayedRate < rate) delayedRate = rate;

	// ---------------------------
	// ★ 外枠（黒 + 蛍光グリーン）
	// ---------------------------
	DrawBox(left - 3, top - 3, left + W + 3, top + H + 3,
		GetColor(0, 0, 0), TRUE);

	// 外枠の発光縁取り
	DrawBox(left - 2, top - 2, left + W + 2, top + H + 2,
		GetColor(0, 255, 120), FALSE);

	// ---------------------------
	// ★ 背景（暗い灰色）
	// ---------------------------
	DrawBox(left, top, left + W, top + H,
		GetColor(40, 40, 60), TRUE);

	// ---------------------------
	// ★ ダメージ遅延バー（黄色）
	// ---------------------------
	int delayedRight = left + (int)(W * delayedRate);
	DrawBox(left, top, delayedRight, top + H,
		GetColor(255, 220, 80), TRUE);

	// ---------------------------
	// ★ 現在HPバー（グラデーション緑）
	// ---------------------------
	int hpRight = left + (int)(W * rate);

	for (int i = 0; i < H; i++)
	{
		float t = (float)i / H;
		int r = (int)(60 + 80 * t);   // 緑→黄緑
		int g = (int)(200 + 40 * t);
		int b = 40;

		DrawLine(left, top + i, hpRight, top + i,
			GetColor(r, g, b));
	}

	// ---------------------------
	// ☆ 白いハイライト線（キラッと感）
	// ---------------------------
	DrawLine(left, top, hpRight, top,
		GetColor(255, 255, 255));
}


void Enemy::FireToPlayer()
{
	player_ = SceneManager::GetInstance().GetPlayer();

	// player_未取得の場合動かさない
	if (player_ == nullptr)
	{
		return;
	}

	// プレイヤー座標
	VECTOR playerPos = player_->GetTransform().GetPos();

	// 弾の方向（敵→プレイヤー）
	VECTOR dir = VSub(playerPos, transform_.pos);
	dir = VNorm(dir);

	// 敵の向きは見た目用に回転
	float targetY = atan2f(dir.x, dir.z);
	float rotateSpeed = 0.05f;
	float delta = targetY - transform_.rot.y;
	while (delta > DX_PI_F) delta -= DX_TWO_PI_F;
	while (delta < -DX_PI_F) delta += DX_TWO_PI_F;
	transform_.rot.y += delta * rotateSpeed;

	// 弾発射位置（敵の中心より少し上）
	VECTOR muzzlePos = transform_.pos;
	muzzlePos.y += 60.0f;

	for (ShotBase* shot : shots_) {
		if (!shot->IsAlive()) {
			shot->CreateShot(muzzlePos, dir);
			return;  // 再利用したので終了
		}
	}

	// 弾作成
	// 弾のモデルをロード
	//shotStraight = resMng_.Load(ResourceManager::SRC::SHOTSTRAIGHT).handleId_;
	ShotBase* s = new ShotStraight(ShotBase::TYPE::STRAIGHT, shot_);
	s->CreateShot(muzzlePos, dir);
	shots_.push_back(s);
}

void Enemy::Damage(int damage)
{
	hp_ -= damage;

	// HPバーと同じ位置をベースにする
	VECTOR world = transform_.pos;
	world.y += 520.0f;  // HPバー位置
	world.z += 50.0f;   // HPバーと同じくらい

	DamagePopup pop;
	pop.pos = world;
	pop.value = (float)damage;
	pop.timer = 1.0f;
	pop.alpha = 1.0f;
	pop.vy = 0.6f;
	pop.vxOffset = (GetRand(100) - 50) * 0.02f;
	damagePopups_.push_back(pop);

	// ダメージフラッシュ
	damageFlashTimer_ = 0.15f;

	if (hp_ <= 0)
	{
		hp_ = 0;
		isAlive_ = false;
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CLEAR);
	}
}

void Enemy::Coliision()
{

}

void Enemy::UpdateProcess(void)
{
	// 移動操作
	ProcessMove();
}

void Enemy::UpdateProcessPost(void)
{
}
