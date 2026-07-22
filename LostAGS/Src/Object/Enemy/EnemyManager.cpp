#include "EnemyManager.h"
#include "EnemyNormal.h"
#include "EnemyBase.h"
#include "EnemyPolice.h"
#include "../Stage/Stage.h"
#include "../../Manager/SoundManager.h"

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

	EnemyBase::ResetChasingEnemyCount();

	isPoliceSpawned_ = false;




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

		{ -3200.0f, -98.0f,  -250.0f }, // 30
		{ -3200.0f, -98.0f,  740.0f }, // 31
		{ -2610.0f, -98.0f,  -250.0f }, // 32
		{ -2610.0f, -98.0f,  740.0f }, // 33
		{ -2000.0f, -98.0f,  -250.0f }, // 34
		{ -2000.0f, -98.0f,  740.0f }, // 35
		{ -2610.0f, -98.0f,  -940.0f }, // 36
		{ -2000.0f, -98.0f,  -940.0f }, // 37

	};


	std::vector<std::vector<int>> links =
	{
	{ 1, 2, 3, 7 },     // 0
	{ 0, 4, },  // 1
	{ 7, 8, 9 },        // 2
	{ 0, 8 },        // 3
	{ 5, 7, 12, 34 },     // 4
	{ 4, 6, 12 },        // 5
	{ 4, 5 },        // 6
	{ 0, 4, 8, 34 },        // 7
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
	{ 26, 30, 36 }, // 29
	{ 29, 31, 32 }, // 30
	{ 30 }, // 31
	{ 30, 33, 36, 34 }, // 32
	{ 32 }, // 33
	{ 4, 32, 35, 37 }, // 34
	{ 34 }, // 35
	{ 26, 29, 32, 37 }, // 36
	{ 26, 29, 36, 34 }, // 37

	};

	debugPatrolPoints_ = points;
	debugPatrolLinks_ = links;


	
	//enemy2->SetPatrolPoints(points);
	//enemy2->SetPatrolLinks(links);
	//enemy2->SetPos(points[4]);
	//enemies_.push_back(enemy2);


	/*for (int i = 0; i < 5; i++)
	{
		EnemyNormal* enemy = new EnemyNormal();

		enemy->Init();

		enemy->SetPatrolPoints(points);
		enemy->SetPatrolLinks(links);

		enemies_.push_back(enemy);
	}*/

	CreatePoliceEnemies();

}
void EnemyManager::Update(Player* player)
{
	VECTOR cameraPos = GetCameraPosition();
	VECTOR cameraTarget = GetCameraTarget();

	/*SoundManager::GetInstance().Set3DListener(
		cameraPos,
		cameraTarget
	);*/

	// =========================
	// 緊急脱出モードになったら警察敵を一度だけ出す
	// =========================
	if (stage_ != nullptr)
	{
		if (stage_->IsEmergencyEscape() && !isPoliceSpawned_)
		{
			isPoliceSpawned_ = true;
			SpawnPoliceEnemies();
		}
	}

	for (auto enemy : enemies_)
	{
		if (enemy == nullptr)
		{
			continue;
		}

		if (!enemy->IsActive())
		{
			continue;
		}

		enemy->Update(player);
	}
}

void EnemyManager::Draw(void)
{


	for (auto enemy : enemies_)
	{
		if (enemy == nullptr)
		{
			continue;
		}

		if (!enemy->IsActive())
		{
			continue;
		}

		// 遮蔽判定をせず、Zバッファに任せる
		enemy->Draw();
	}

#ifdef _DEBUG

	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetUseLighting(FALSE);
	SetUseBackCulling(FALSE);

	// 現在の巡回ルートなどのデバッグ描画

	SetUseBackCulling(TRUE);
	SetUseLighting(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

#endif
}

void EnemyManager::SetStage(Stage* stage)
{
	stage_ = stage;
}


void EnemyManager::SpawnPoliceEnemies(void)
{
	for (auto police : policeEnemies_)
	{
		if (police == nullptr)
		{
			continue;
		}

		police->SetActive(true);
	}
}

void EnemyManager::CreatePoliceEnemies(void)
{
	std::vector<VECTOR> spawnPoints =
	{
		{ -4240.0f, -98.0f,  900.0f },
		{ -4240.0f, -98.0f,  900.0f },
		{ -4240.0f, -98.0f,  900.0f },
	};

	for (int i = 0; i < 3; i++)
	{
		EnemyPolice* police = new EnemyPolice();

		police->Init();

		police->SetPatrolPoints(debugPatrolPoints_);
		police->SetPatrolLinks(debugPatrolLinks_);

		police->SetPos(spawnPoints[i]);

		// 最初は非表示・非更新
		police->SetActive(false);

		enemies_.push_back(police);
		policeEnemies_.push_back(police);
	}
}
