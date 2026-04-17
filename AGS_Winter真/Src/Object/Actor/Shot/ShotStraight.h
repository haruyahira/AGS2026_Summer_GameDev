#pragma once

#include "ShotBase.h"

class ShotStraight : public ShotBase
{
public:
	// コンストラクタ
	ShotStraight(TYPE type, int baseModelId);
	// デストラクタ
	~ShotStraight(void);

protected:
	// コピーコンストラクタ
	ShotStraight() = default;
	// パラメータ設定
	void SetParam(void) override;
};