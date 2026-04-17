#pragma once

#include <DxLib.h>
#include <vector>
#include "CharactorBase.h"

class Player;
class ShotBase;
class ColliderSphere;

class Enemy : public CharactorBase
{
public:
	struct DamagePopup
	{
		VECTOR pos;      // ワールド座標
		float value;     // ダメージ数値
		float timer;     // 生存時間
		float alpha;     // 透明度
		float vy;        // 上に動くスピード
		float vxOffset;   // ← 横揺れ用
	};
	std::vector<DamagePopup> damagePopups_;
	// 敵弾リスト取得
	std::vector<ShotBase*>& GetShots() { return shots_; }
	// 視野の広さ
	static constexpr float VIEW_RANGE = 1000.0f;
	// 視野角
	static constexpr float VIEW_ANGLE = 30.0f;

	static constexpr  float HEARING_RANGE = 400.0f; // 聴覚による索敵範囲

	// エネミーの攻撃間隔
	static constexpr float TERM_ATTACK = 3.0f;

	Enemy();
	~Enemy(void) = default;

	// 更新
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// リソースロード
	void InitLoad(void) override;

	// Transform初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void)override;
	
	void Damage(int damage);
	void Coliision(void);

	void DrawModelOnly();

	ColliderSphere* GetHitCollider() const { return collider_; }
protected:
	// 更新系
	virtual void UpdateProcess(void) override;
	virtual void UpdateProcessPost(void) override;

private:
	// プレイヤー
	Player* player_;

	ColliderSphere* collider_ = nullptr;

	// 弾の座標を取得
	const std::vector<ShotBase*>& GetShot(void) const { return shots_; }

	// 検知フラグ
	bool isNotice_;
	bool isHear_;
	bool isAttack_;

	// HP
	int hp_;
	int maxHp_;
	// 弾ハンドル
	int shotStraight;
	int shot_;
	int damageFont_;  // ダメージ数字フォント

	VECTOR dirEnemy;
	VECTOR diff;
	VECTOR ddirPlayerFromEnemy;


	// 攻撃方法
	// プレイヤーの方向に発射
	void FireToPlayer();
	// 移動
	void ProcessMove(void);

	// HPゲージ
	void DrawFancyHpBar();

	// 弾
	std::vector<ShotBase*> shots_;

	float cnt = 0.0f; // 消す
	float damageFlashTimer_;


	float fireInterval_ = 120.0f; // 発射間隔（フレーム数）2秒に1発くらい
	float fireTimer_ = 0.0f;

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, 0.0f, 0.0f };
	// 衝突判定用線分開始(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_START_LOCAL_POS = { 0.0f, 130.0f, 0.0f };
	// 衝突判定用線分終了(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_END_LOCAL_POS = { 0.0f, 50.0f, 0.0f };
};

