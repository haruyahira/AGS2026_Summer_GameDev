#include <vector>
#include <map>
#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "WarpStar.h"
#include "Player.h"
#include "Planet.h"
#include "Collider/Collider.h"
#include "Common/Transform.h"
#include "Stage.h"
#include "Furniture/Table.h"

Stage::Stage(Player* player)
	: resMng_(ResourceManager::GetInstance())
{
	player_ = player;
	activeName_ = NAME::FIRST_STAGE;
	step_ = 0.0f;
}

Stage::~Stage(void)
{

	//// ワープスター
	//for (auto star : warpStars_)
	//{
	//	delete star;
	//}
	//warpStars_.clear();

	// 惑星
	for (auto pair : stages_)
	{
		delete pair.second;
	}
	stages_.clear();

	// 家具の削除
	for (auto f : furnitures_) {
		delete f;
	}
	furnitures_.clear();

}

void Stage::Init(void)
{
	MakeMainStage();
	//MakeWarpStar();

	step_ = -1.0f;
}

void Stage::Update(void)
{

	//// ワープスター
	//for (const auto& s : warpStars_)
	//{
	//	s->Update();
	//}

	// 惑星
	for (const auto& s : stages_)
	{
		s.second->Update();
	}

	// 家具の更新
	for (auto f : furnitures_) {
		f->Update();
	}
}

void Stage::Draw(void)
{

	//// ワープスター
	//for (const auto& s : warpStars_)
	//{
	//	s->Draw();
	//}

	// 惑星
	for (const auto& s : stages_)
	{
		s.second->Draw();
	}

	//家具の描画
	for (auto f : furnitures_) {
		f->Draw();
	}
}

void Stage::ChangeStage(NAME type)
{

	activeName_ = type;

	// 対象のステージを取得する
	activePlanet_ = GetPlanet(activeName_);

	// ステージの当たり判定をプレイヤーに設定
	player_->ClearCollider();
	player_->AddCollider(activePlanet_->GetTransform().collider);

	step_ = TIME_STAGE_CHANGE;

}

Planet* Stage::GetPlanet(NAME type)
{
	if (stages_.count(type) == 0)
	{
		return nullPlanet;
	}

	return stages_[type];
}

void Stage::MakeMainStage(void)
{

	// 最初のステージ
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_PLANET));
	planetTrans.scl = { 10.0f, 10.0f, 10.0f };
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { 0.0f, -100.0f, 0.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	NAME name = NAME::FIRST_STAGE;
	Planet* stage =
		new Planet(
			name, Planet::TYPE::GROUND, planetTrans);
	stage->Init();
	// 生成したステージをリストに登録
	stages_.emplace(name, stage);
	//------------------------------------------------------------------------------



	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 2; j++) {
		// 1台目: 5.0f  /  2台目: 155.0f  /  3台目: 305.0f
		float posZ = 5.0f +(i * 70.0f);
		float posX;
		if (skipNumbers_.count(i) > 0) continue;
		CreateFurniture({
			ResourceManager::SRC::F_TABLE,
			{ 5.0f, -100.0f, posZ }, // X座標だけループで右にずらしていく
			{ 0.5f,  0.5f,   0.5f },
			{ 0.0f,  0.0f,   0.0f }
			});
	}
	}
}

//void Stage::MakeWarpStar(void)
//{
//
//	Transform trans;
//	WarpStar* star;
//
//	// 落とし穴惑星へのワープスター
//	//------------------------------------------------------------------------------
//	trans.pos = { -910.0f, 200.0f, 894.0f };
//	trans.scl = { 0.6f, 0.6f, 0.6f };
//	trans.quaRot = Quaternion::Euler(
//		AsoUtility::Deg2RadF(-25.0f),
//		AsoUtility::Deg2RadF(-50.0f),
//		AsoUtility::Deg2RadF(0.0f)
//	);
//
//	star = new WarpStar(player_, trans);
//	star->Init();
//	warpStars_.push_back(star);
//	//------------------------------------------------------------------------------
//
//}

void Stage::CreateFurniture(const FurnitureData& data)
{
	Transform trans;
	// 1. 引数 data からモデル、座標、スケールを設定
	trans.SetModel(resMng_.LoadModelDuplicate(data.modelSrc));
	trans.pos.x = data.pos.x;
	trans.pos.y = data.pos.y;
	trans.pos.z = data.pos.z;


	trans.scl.x = data.scl.x;
	trans.scl.y = data.scl.y;
	trans.scl.z = data.scl.z;

	// 回転の設定（data.rot を使用）
	trans.quaRot = Quaternion::Euler(data.rot.x, data.rot.y, data.rot.z);

	// 2. コライダの設定（コメントアウトを外せば有効になります）
	//trans.MakeCollider(Collider::TYPE::MESH);
	trans.Update();

	// 3. Furnitureインスタンスの生成
	// 第2引数は上で設定した 'trans' を渡します
	//Furniture* f = new Furniture(NAME::INTERIOR, &trans);
	Table* f = new Table(&trans);
	f->Init();

	// 4. Stageクラスのリストに追加
	// objects_ もしくは furnitures_ など、ヘッダーで定義した名前に合わせます
	player_->AddFurniture(f);

	// 4. Stageクラスのリストに追加
	furnitures_.push_back(f);
}