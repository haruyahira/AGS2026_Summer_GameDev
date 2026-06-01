#include "EnemyManager.h"
#include "EnemyNormal.h"

EnemyManager::EnemyManager(void) {}

EnemyManager::~EnemyManager(void)
{
	// メモリリーク防止のため、管理している敵をすべて削除
	for (auto enemy : enemies_)
	{
		delete enemy;
	}
	enemies_.clear();
}

void EnemyManager::Init(void)
{
	// テスト用に、普通の敵(EnemyNormal)を1体生成して配列に追加
	EnemyNormal* testEnemy = new EnemyNormal();
	testEnemy->Init();
	EnemyNormal* enemy2 = new EnemyNormal();
	enemy2->Init();



	std::vector<VECTOR> points =
	{
		{ -1400.0f, -98.0f, 760.0f }, // 0
		{ -1364.0f, -98.0f, -1164.0f }, // 1
		{ 13, -98.0f, -1120.0f }, // 2
		{ -171.0f, -98.0f,  760.0f }, // 3
		{ -1618.0f, -98.0f,  -328.0f }, // 4
		{ -1618.0f, -98.0f,  712.0f }, // 5
		{ -1618.0f, -98.0f,  -1124.0f }, // 6
		{ -1200.0f, -98.0f,  -314.0f }, // 7
		{ -171.0f, -98.0f,  -314.0f }, // 8
	};


	std::vector<std::vector<int>> links =
	{
	{ 1, 2, 3, 7 },     // 0
	{ 0, 4, },  // 1
	{ 7, 8 },        // 2
	{ 0, 8 },        // 3
	{ 5, 7 },     // 4
	{ 4, 6 },        // 5
	{ 4, 5 },        // 6
	{ 0, 4, 8 },        // 7
	{ 2, 3, 7 },        // 8

	};


	testEnemy->SetPatrolPoints(points);
	testEnemy->SetPatrolLinks(links);
	testEnemy->SetPos(points[5]);
	enemies_.push_back(testEnemy);
	


	enemy2->SetPatrolPoints(points);
	enemy2->SetPatrolLinks(links);
	enemy2->SetPos(points[4]);
	enemies_.push_back(enemy2);


	for (int i = 0; i < 5; i++)
	{
		EnemyNormal* enemy = new EnemyNormal();

		enemy->Init();

		enemy->SetPatrolPoints(points);
		enemy->SetPatrolLinks(links);

		enemies_.push_back(enemy);
	}


}

void EnemyManager::Update(Player* player)
{
	// すべての敵のUpdateをループで回す（今回は中身空っぽですが構造として）
	for (auto enemy : enemies_)
	{
		if (enemy) enemy->Update(player);
	}
}

void EnemyManager::Draw(void)
{
	// すべての敵のDrawをループで回して描画する
	for (auto enemy : enemies_)
	{
		if (enemy) enemy->Draw();
	}


}