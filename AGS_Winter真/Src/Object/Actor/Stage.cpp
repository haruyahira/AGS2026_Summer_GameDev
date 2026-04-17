#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Utility/AsoUtility.h"
#include "../Collider/ColliderModel.h"
#include "Stage.h"
Stage::Stage(void)
{

}


Stage::~Stage(void)
{
}


void Stage::Update(void)
{
	
}

void Stage::Draw(void)
{
	MV1DrawModel(transform_.modelId);

	//DrawFormatString(
	//	0, 50, 0xffffff,
	//	"座標　 ：(% .1f, % .1f, % .1f),\n",
	//	transform_.pos.x,
	//	transform_.pos.y,
	//	transform_.pos.z
	//);
}

void Stage::Release(void)
{

}

void Stage::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(
		ResourceManager::SRC::STAGE).handleId_);
}

void Stage::InitTransform(void)
{
	transform_.scl = STAGE_SCL;
	transform_.pos = STAGE_POS;
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();

	transform_.Update();
}

void Stage::InitCollider(void)
{
	// DxLib側の衝突情報セットアップ
	MV1SetupCollInfo(transform_.modelId);
	// モデルのコライダ
	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Stage::InitAnimation(void)
{
}

void Stage::InitPost(void)
{
}

