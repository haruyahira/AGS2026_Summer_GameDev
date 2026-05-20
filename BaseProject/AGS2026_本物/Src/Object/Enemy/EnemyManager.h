#pragma once
#include <DxLib.h>
#include <vector>

class EnemyBase;

class EnemyManager
{
public:
	EnemyManager(void);
	~EnemyManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);

private:
	std::vector<EnemyBase*> enemies_; // “G‚ğ‚Ü‚Æ‚ß‚ÄŠÇ—‚·‚é”z—ñ
};