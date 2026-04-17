#pragma once
#include "ActorBase.h"

class Stage : public ActorBase
{

public:

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL = 0,
		MAX,
	};

	static constexpr VECTOR STAGE_SCL = { 2.0f,2.0f,2.0f };  // ステージの大きさ
	static constexpr VECTOR STAGE_POS = { 0.0f, -25.1f, 0.0f };  // ステージの場所

	Stage(void);
	~Stage(void);

	// 更新
	void Update(void) override;

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

protected:

private:
	
	
};
