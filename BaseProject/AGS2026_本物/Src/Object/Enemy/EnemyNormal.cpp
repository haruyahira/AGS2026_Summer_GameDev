#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "EnemyNormal.h"

EnemyNormal::EnemyNormal(void) : EnemyBase()
{
}

EnemyNormal::~EnemyNormal(void)
{
	
}

void EnemyNormal::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::ENEMYNORMAL));
	//MV1SetAmbColorScale(transform_.modelId, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	transform_.scl = { 0.1f, 0.1f, 0.1f };
	transform_.pos = { 0.0f, -30.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();
}

void EnemyNormal::Update(void)
{
	// 生成段階ではまだ動かさないので空っぽでOKです
}

void EnemyNormal::Draw(void)
{
	// モデルの描画
	MV1DrawModel(transform_.modelId);
}