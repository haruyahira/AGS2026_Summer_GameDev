#pragma once

#include "ShotBase.h"

class Shot : public ShotBase
{
public:
	// コンストラクタ
	Shot(TYPE type, int baseModelId);
	// デストラクタ
	~Shot(void);

protected:
	// コピーコンストラクタ
	Shot() = default;
	// パラメータ設定
	void SetParam(void) override;

	// ★ 魔法弾用の描画
	void DrawModel(void) override;
};