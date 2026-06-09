#pragma once
#include <DxLib.h>
#include <vector>

class EnemyBase;
class Player;

class EnemyManager
{
public:
	EnemyManager(void);
	~EnemyManager(void);

	void Init(void);
	void Update(Player* player);
	void Draw(void);

private:
	std::vector<EnemyBase*> enemies_; // “G‚ğ‚Ü‚Æ‚ß‚ÄŠÇ—‚·‚é”z—ñ
private:
	std::vector<VECTOR> debugPatrolPoints_;
	std::vector<std::vector<int>> debugPatrolLinks_;

	bool isDrawDebugPatrol_ = true;
};