#pragma once
#include <map>
#include <memory>
#include <DxLib.h>
#include "ActorBase.h"
class AnimationController;
class Collider;
class Capsule;
class Furniture;

class Player : public ActorBase
{

public:

	// スピード
	static constexpr float SPEED_MOVE = 3.0f;
	static constexpr float SPEED_RUN = 5.0f;

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;

	// ジャンプ力
	static constexpr float POW_JUMP = 35.0f;

	// ジャンプ受付時間
	static constexpr float TIME_JUMP_IN = 0.3f;

	// 当たり判定部位
	enum class BONE_PART
	{
		BODY,      // 体（Spine ～ Head）
		LEFT_ARM,  // 左腕（肩 ～ 手首）
		RIGHT_ARM, // 右腕（肩 ～ 手首）
		LEFT_LEG,  // 左足（太もも ～ 足首）
		RIGHT_LEG, // 右足（太もも ～ 足首）
	};

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		PRONE,
		WARP_RESERVE,
		WARP_MOVE,
		DEAD,
		VICTORY,
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		PRONE_IDLE,
		PRONE_WALK,
		PRONE_RUN,
		WARP_PAUSE,
		FLY,
		FALLING,
		VICTORY
	};

	// 懐中電灯（スポットライト）の構造体
	struct Flashlight {
		int handle = -1;
		bool isOn = true;
		float range = 1000.0f;
		float outerAngle = 0.4f;
		float innerAngle = 0.2f;
	} flashlight_;

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	// 衝突判定に用いられるコライダ制御
	void AddCollider(Collider* collider);
	void ClearCollider(void);

	// 衝突用カプセルの取得
	const Capsule* GetCapsule(void) const;

	// 頭の座標を返す関数
	VECTOR GetHeadPosition() const {
		// "Head"という名前のフレームを探してその座標を返す
		// 名前はモデルによって違うので注意（"Head"、"頭"、"Neck"など）
		int headFrame = MV1SearchFrame(transform_.modelId, "Head");
		return MV1GetFramePosition(transform_.modelId, headFrame);
	}

	void AddFurniture(Furniture* furniture) {
		furnitures_.push_back(furniture);
	}
private:

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;

	// 状態管理
	STATE state_;

	// アニメーション種別
	ANIM_TYPE animType_;

	// 移動スピード
	float speed_;

	// 移動方向
	VECTOR moveDir_;

	// 移動量
	VECTOR movePow_;

	// 移動後の座標
	VECTOR movedPos_;

	// 頭の座標
	VECTOR headPos_;
	VECTOR worldHeadPos_;
	VECTOR localHeadPos_; 

	// 回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// ジャンプ量
	VECTOR jumpPow_;

	// ジャンプ判定
	bool isJump_;
	bool isStand_;

	// ジャンプの入力受付時間
	float stepJump_;
	float standHeight_;

	// 衝突判定に用いられるコライダ
	std::vector<Collider*> colliders_;
	std::vector<Furniture*> furnitures_;
	Capsule* capsule_;
	Furniture* furniture;
	
	// capsule_ の代わりに map（または vector）で管理
	std::map<BONE_PART, Capsule*> capsules_;

	// 衝突チェック
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// 丸影
	int imgShadow_;
	int headFrame_;
	int leftShoulderFrame_;
	int leftHandFrame_;
	int rightShoulderFrame_;
	int rightHandFrame_;
	int proneFrame_;
	// 頭のボーンフレーム
	int headBoneFrame_;
	// 足元のボーンフレーム
	int SpineFrame_;
	float pRadius_;

	// アニメーションの初期化
	void InitAnimation(void);
	// 当たり判定の初期化
	void InitCollider(void);
	void InitFlashLight(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateProne(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateProne(void);
	void UpdateCommon(void);
	void UpdateFlashLight(void);
	
	// 描画系
	void DrawShadow(void);

	// 操作
	void ProcessMove(void);
	void ProcessJump(void);

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// 衝突判定
	void Collision(void);
	void CollisionGravity(void);
	void CollisionCapsule(void);
	void CollisionBox(void);

	// 移動量の計算
	void CalcGravityPow(void);

	// 着地モーション終了
	bool IsEndLanding(void);

	// しゃがみ状態か
	bool IsProne() const { return state_ == STATE::PRONE; }

	// カメラ
	void SetFirstPerson(void);

};
