#pragma once
#include "../ActorBase.h"

class EnemyBase : public ActorBase
{
public:
	EnemyBase(void);
	virtual ~EnemyBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;
};