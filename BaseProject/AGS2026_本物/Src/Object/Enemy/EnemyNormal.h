#pragma once
#include "EnemyBase.h"

class EnemyNormal : public EnemyBase
{
public:
	EnemyNormal(void);
	virtual ~EnemyNormal(void);

	virtual void Init(void) override;
	virtual void Update(void) override;
	virtual void Draw(void) override;
};