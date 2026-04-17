#pragma once
#include "CharactorBase.h"
#include "../../../Manager/SceneManager.h"

class Katana : public CharactorBase
{

public:
	static constexpr VECTOR KATANA_SCL = { 0.5f,0.5f,0.5f };  // プレイヤーの大きさ

	static constexpr VECTOR PLAYER_POS = { 0.0f, 0.0f, 0.0f };  // プレイヤーの初期位置


	Katana(void);
	~Katana(void);

	// 描画
	void Draw(void) override;

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

	void UpdateHandMatrix(const MATRIX& handMatrix);

protected:
	// 更新系
	virtual void UpdateProcess(void) override;
	virtual void UpdateProcessPost(void) override;

private:
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

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, 0.0f, 0.0f };
	// 衝突判定用線分開始(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_START_LOCAL_POS = { 0.0f, 130.0f, 0.0f };
	// 衝突判定用線分終了(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_END_LOCAL_POS = { 0.0f, 50.0f, 0.0f };


	// 操作
	void ProcessMove(void);
	void ProcessJump(void);

	



};
