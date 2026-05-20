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

	enemies_.push_back(testEnemy);
}

void EnemyManager::Update(void)
{
	// すべての敵のUpdateをループで回す（今回は中身空っぽですが構造として）
	for (auto enemy : enemies_)
	{
		if (enemy) enemy->Update();
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