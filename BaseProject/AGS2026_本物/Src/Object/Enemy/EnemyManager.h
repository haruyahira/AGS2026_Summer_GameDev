#pragma once
#include <DxLib.h>
#include <vector>

class EnemyBase;
class Stage;
class Player;

class EnemyManager
{
public:
	EnemyManager(void);
	~EnemyManager(void);

	void Init(void);
	void Update(Player* player);
	void Draw(void);
	void SetStage(Stage* stage);
	bool IsAnyEnemyChasing();

private:
	std::vector<EnemyBase*> enemies_; // “G‚ğ‚Ü‚Æ‚ß‚ÄŠÇ—‚·‚é”z—ñ
private:
	std::vector<VECTOR> debugPatrolPoints_;
	std::vector<std::vector<int>> debugPatrolLinks_;
	Stage* stage_ = nullptr;
	bool isDrawDebugPatrol_ = true;
};