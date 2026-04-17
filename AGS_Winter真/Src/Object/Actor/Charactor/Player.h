#pragma once
#include "../../../Manager/SceneManager.h"
#include "../../Common/CutIn.h"
#include "CharactorBase.h"
#include "Katana.h"
#include "Flash.h"
#include "FlashExplosion.h"
#include "../Shot/ShotSword.h"
#include "../Shot/Shot.h"
#include "Enemy.h"
class ShotBase;


class Player : public CharactorBase
{

public:


	static constexpr VECTOR PLAYER_SCL = { 0.4f,0.4f,0.4f };  // プレイヤーの大きさ

	static constexpr VECTOR PLAYER_POS = { 0.0f, 0.0f, 0.0f };  // プレイヤーの初期位置
	

	Player(void);
	~Player(void);

	// 描画
	void Draw(void) override;

	void DrawModelOnly();

	void DrawShots();

	void DrawFancyHpBar();

	// 解放
	void Release(void)override;

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

	void ResetState();

	ColliderSphere* GetBulletHitCollider() const { return bulletHitCollider_; }

protected:
	// 更新系
	virtual void UpdateProcess(void) override;
	virtual void UpdateProcessPost(void) override;

private:
	Katana* katana_;

	Enemy* enemy_;

	CutIn cutIn;

	// 移動速度(通常)
	static constexpr float SPEED_MOVE = 9.0f;
	// 移動速度(ダッシュ)
	static constexpr float SPEED_DASH = 14.514f;
	// ジャンプ力
	static constexpr float POW_JUMP = 6000.0f;

	int nowFootstepSE_ = -1;

	// 刀
	int sowrdId_;
	int sowrdIndex_;
	int rightHandBone_ = -1;
	int NEffectHandle_ = -1;

	int shot_;

	// シェイダー
	int toonPS_;
	int outlinePS_;

	// フラグ
	bool isWalk_;
	bool isRun_;
	bool isNAttack_;

	// 必殺技
	bool isSpecial_ = false;
	bool isD_ = false;
	float specialTimer_ = 0.0f;
	const float SPECIAL_TIME = 1.5f;
	ColliderSphere* bulletHitCollider_ = nullptr;
  
	std::vector<ShotBase*> shots;
	std::vector<Flash*> flashes_;
	std::vector<FlashExplosion*> explosions_;  // ★追加：爆発エフェクト
	
	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, 0.0f, 0.0f };
	// 衝突判定用線分開始(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_START_LOCAL_POS ={ 0.0f, 130.0f, 0.0f };
	// 衝突判定用線分終了(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_END_LOCAL_POS = { 0.0f, 50.0f, 0.0f };

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 操作
	void ProcessMove(void);
	void ProcessJump(void);

	// 通常攻撃　左クリック
	void NormalAttack(void);
	// バリア　右クリック
	void Defense(void);
	// スキル E
	void MediumAttack(void);
	// 必殺技 Q
	void SpecialAttack(void);
	bool isCutIn_ = false;
	int cutInTimer_ = 0;
	int flashAlpha_ = 0;
	int cutInHandle_ = -1;
	bool specialActive_ = false;
	VECTOR GetRightHandWorldPos();

	


	int h;
	int hp_;
	int hpMax_;
	float hpDelayedRate_;     // HP遅延バー
	float spGauge_;           // 必殺ゲージ（0〜100）
	float spDelayedRate_;     // 必殺遅延ゲージ

	float explosionTimer_ = 0.0f;
	bool explosionDone_ = false;

};
