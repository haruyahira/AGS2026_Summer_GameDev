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
		{ 13.0f, -98.0f, -1120.0f }, // 2
		{ -171.0f, -98.0f,  760.0f }, // 3
		{ -1618.0f, -98.0f,  -328.0f }, // 4
		{ -1618.0f, -98.0f,  712.0f }, // 5
		{ -1618.0f, -98.0f,  -1124.0f }, // 6
		{ -1200.0f, -98.0f,  -314.0f }, // 7
		{ -171.0f, -98.0f,  -314.0f }, // 8
		{ -1025.0f, -98.0f,  -1164.0f }, // 9
		{ -470.0f, -98.0f,  760.0f }, // 10

		{ -2525.0f, -98.0f,  900.0f }, // 11
		{ -1618.0f, -98.0f,  900.0f }, // 12
		{ -1618.0f, -98.0f,  1180.0f }, // 13
		{ -660.0f, -98.0f,   1180.0f }, // 14
		{ -2525.0f, -98.0f,  1180.0f }, // 15
		{ -2140.0f, -98.0f,  1180.0f }, // 16
		{ -3390.0f, -98.0f,  1180.0f }, // 17
		{ -3600.0f, -98.0f,  900.0f }, // 18

		{ -3600.0f, -98.0f,  1350.0f }, // 19
		{ -4240.0f, -98.0f,  1350.0f }, // 20
		{ -4240.0f, -98.0f,  900.0f }, // 21
		{ -4240.0f, -98.0f,  -80.0f }, // 22
		{ -3600.0f, -98.0f,  -80.0f }, // 23
		{ -4240.0f, -98.0f,  -510.0f }, // 24
		{ -3600.0f, -98.0f,  -510.0f }, // 25
		{ -3600.0f, -98.0f,  -920.0f }, // 26
		{ -4240.0f, -98.0f,  -1140.0f }, // 27
		{ -3600.0f, -98.0f,  -1140.0f }, // 28
		{ -3200.0f, -98.0f,  -920.0f }, // 29
		{ -1900.0f, -98.0f,  -920.0f }, // 30


		//{ -3600.0f, -98.0f,  900.0f }, // 29
		//{ -3600.0f, -98.0f,  900.0f }, // 30
		//{ -3600.0f, -98.0f,  900.0f }, // 31
		//{ -3600.0f, -98.0f,  900.0f }, // 32
		//{ -3600.0f, -98.0f,  900.0f }, // 33
	

	};


	std::vector<std::vector<int>> links =
	{
	{ 1, 2, 3, 7 },     // 0
	{ 0, 4, },  // 1
	{ 7, 8, 9 },        // 2
	{ 0, 8 },        // 3
	{ 5, 7, 12 },     // 4
	{ 4, 6, 12 },        // 5
	{ 4, 5 },        // 6
	{ 0, 4, 8 },        // 7
	{ 2, 3, 7, 9 },        // 8
	{ 2, 8 }, // 9
	{ 0, 3, 7, 9 }, // 10
	{ 12, 18 }, // 11
	{ 5, 11, 13 }, // 12
	{ 12, 14 }, // 13
	{ 13 }, // 14
	{ 11, 16, 17 }, // 15
	{ 15 }, // 16
	{ 15 }, // 17
	{ 11, 19, 20, 21, 22, 23 }, // 18
	{ 18, 20, 21 }, // 19
	{ 18, 19, 21, 23 }, // 20
	{ 18, 19, 20, 22, 23 }, // 21
	{ 18, 24 }, // 22
	{ 18, 19, 20, 21, 22 }, // 23
	{ 22, 26 }, // 24
	{ 24, 26, 28 }, // 25
	{ 24, 25, 27, 28, 29 }, // 26
	{ 24, 26, 28 }, // 27
	{ 25, 26, 27, }, // 28
	{ 26, 30 }, // 29
	{ 29 }, // 30

	};


	testEnemy->SetPatrolPoints(points);
	testEnemy->SetPatrolLinks(links);
	testEnemy->SetPos(points[5]);
	enemies_.push_back(testEnemy);


	debugPatrolPoints_ = points;
	debugPatrolLinks_ = links;
	


	//enemy2->SetPatrolPoints(points);
	//enemy2->SetPatrolLinks(links);
	//enemy2->SetPos(points[4]);
	//enemies_.push_back(enemy2);


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


	// デバッグ：巡回ポイント番号表示
	if (isDrawDebugPatrol_)
	{
		for (int i = 0; i < (int)debugPatrolPoints_.size(); i++)
		{
			VECTOR pos = debugPatrolPoints_[i];

			// 少し上に表示する
			pos.y += 40.0f;

			// 3D座標を画面座標に変換
			VECTOR screenPos = ConvWorldPosToScreenPos(pos);

			// 画面内・カメラ前だけ表示
			if (screenPos.z >= 0.0f && screenPos.z <= 1.0f)
			{
				DrawFormatString(
					(int)screenPos.x,
					(int)screenPos.y,
					GetColor(255, 255, 0),
					"%d",
					i
				);
			}
		}
	}

}