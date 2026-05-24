#include <vector>
#include <map>
#include <DxLib.h>
#include <set>
#include <assert.h>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/ResourceManager.h"
#include "WarpStar.h"
#include "../Player.h"
#include "Planet.h"
#include "../Collider/Collider.h"
#include "../Common/Transform.h"
#include "Stage.h"
#include "../Furniture/Table.h"
#include "../Furniture/Wall.h"
#include "../Furniture/Showcase.h"
#include "../Furniture/Ceiling.h"

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

	for (auto f : glassFurnitures_) {
		delete f;
	}
	glassFurnitures_.clear();


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


	for (auto f : glassFurnitures_) {
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



	// ガラス家具は最後に描画
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);

	MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_ALWAYS);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	for (auto f : glassFurnitures_) {
		f->Draw();
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY);

	SetUseBackCulling(TRUE);
	SetWriteZBuffer3D(TRUE);


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
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLOOR));
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


	std::set<int> skipX = { 1,2 };
	std::set<int> skipZ = { 2,3, 6,7 };

	for (int i = 0; i < 4; i++)
	{
		// スキップ番号なら生成しない
		if (skipX.count(i) > 0)continue;


		for (int j = 0; j < 10; j++)
		{
			// スキップ番号なら生成しない
			if (skipZ.count(j) > 0)
				continue;
			// Z方向に並べる
			float posZ = 5.0f + (j * 75.0f);

			// X方向に2列並べる
			// 1列目: 5.0f
			// 2列目: 155.0f
			float posX = 5.0f + (i * -120.0f);

			CreateFurniture({
				ResourceManager::SRC::F_TABLE,
				{ posX, -100.0f, posZ },
				{ 0.5f, 0.5f, 0.5f },
				{ 0.0f, 0.0f, 0.0f } // 回転
				});
		}
	}

	// 壁一覧
	std::vector<FurnitureData> wallDatas =
	{

	{     // ①
		  ResourceManager::SRC::WALL,
		  { 88.0f, -100.0f, -290.0f },
		  { 5.0f, 1.0f, 0.5f },
		  { 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f }
	},
	{
		  ResourceManager::SRC::WALL,
		  { -1700.f, -100.0f, -290.0f },
		  { 5.0f, 1.0f, 0.5f },
		  { 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
	},
	{
		// ショーケース前L
		ResourceManager::SRC::WALL,
		{ -1500.f, -100.0f, -805.0f },
		{ 1.8f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
    },
    {     // ショーケース前R
		ResourceManager::SRC::WALL,
		{ -1500.f, -100.0f, 350.0f },
		{ 2.7f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
    },

    {
	  // L
	  ResourceManager::SRC::WALL,
	  { -1000.0f, -100.0f, -1200.0f },
	  { 5.0f, 1.0f, 0.5f },
	  { 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }

    },
    {
	  // R
	  ResourceManager::SRC::WALL,
	  { -680.0f, -100.0f, 800.0f },
	  { 4.7f, 1.0f, 0.5f },
	  { 0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f }
    },
	};

	// 壁をまとめて生成
	for (const auto& wallData : wallDatas)
	{
		CreateFurniture(wallData);
	}

	// 天井
	CreateFurniture({
	ResourceManager::SRC::FLOOR,
	{ 10.0, 250.0f, 10.0f },
	{ 10.0f, 0.5f, 10.0f },
	{ AsoUtility::Deg2RadF(180.0f), 0.0f, 0.0f}
		});

	// ケーキショーケース本体
	CreateFurniture({
		ResourceManager::SRC::F_F,
		{ -1200.0f, -100.0f, -800.0f },
		{ 1.0f, 0.5f, 1.0f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		});

	// ケーキショーケースガラス
	CreateFurniture({
		ResourceManager::SRC::F_G,
		{ -1200.0f, -100.0f, -800.0f },
		{ 1.0f, 0.5f, 1.0f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		});

};

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

//void Stage::CreateFurniture(const FurnitureData& data)
//{
//	Transform trans;
//	// 1. 引数 data からモデル、座標、スケールを設定
//	trans.SetModel(resMng_.LoadModelDuplicate(data.modelSrc));
//	trans.pos.x = data.pos.x;
//	trans.pos.y = data.pos.y;
//	trans.pos.z = data.pos.z;
//
//	trans.scl.x = data.scl.x;
//	trans.scl.y = data.scl.y;
//	trans.scl.z = data.scl.z;
//
//	// 回転の設定（data.rot を使用）
//	trans.quaRot = Quaternion::Euler(data.rot.x, data.rot.y, data.rot.z);
//
//	// 2. コライダの設定（コメントアウトを外せば有効になります）
//	//trans.MakeCollider(Collider::TYPE::MESH);
//	trans.Update();
//
//	// 3. Furnitureインスタンスの生成
//	// 第2引数は上で設定した 'trans' を渡します
//	//Furniture* f = new Furniture(NAME::INTERIOR, &trans);
//	Furniture* f = nullptr;
//	if (data.modelSrc == ResourceManager::SRC::F_TABLE) {
//		f = new Table(&trans);
//	}
//	else if (data.modelSrc == ResourceManager::SRC::WALL) {
//		f = new Wall(&trans);
//	}
//
//
//	else if (data.modelSrc == ResourceManager::SRC::F_F) {
//		f = new Showcase(&trans);
//	}
//	else if (data.modelSrc == ResourceManager::SRC::F_G) {
//		f = new Showcase(&trans);
//	}
//
//
//
//	f->Init();
//
//	// 4. Stageクラスのリストに追加
//	// objects_ もしくは furnitures_ など、ヘッダーで定義した名前に合わせます
//	player_->AddFurniture(f);
//
//	// 4. Stageクラスのリストに追加
//	furnitures_.push_back(f);
//}

void Stage::CreateFurniture(const FurnitureData& data)
{
	Transform trans;

	trans.SetModel(resMng_.LoadModelDuplicate(data.modelSrc));

	trans.pos.x = data.pos.x;
	trans.pos.y = data.pos.y;
	trans.pos.z = data.pos.z;

	trans.scl.x = data.scl.x;
	trans.scl.y = data.scl.y;
	trans.scl.z = data.scl.z;

	trans.quaRot = Quaternion::Euler(data.rot.x, data.rot.y, data.rot.z);

	// 必要なら当たり判定
	// trans.MakeCollider(Collider::TYPE::MESH);

	trans.Update();

	Furniture* f = nullptr;

	if (data.modelSrc == ResourceManager::SRC::F_TABLE) {
		f = new Table(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::WALL) {
		f = new Wall(&trans, data.rot.y);
		assert(f != nullptr);
	}
	else if (data.modelSrc == ResourceManager::SRC::F_F) {
		f = new Showcase(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::F_G) {
		f = new Showcase(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::FLOOR) {
		f = new Ceiling(&trans, data.rot.y);
	}

	// ここ超重要

	if (f == nullptr) {
		return;
	}

	f->Init();

	// ガラスモデルだけDXLib側でも半透明化
	if (data.modelSrc == ResourceManager::SRC::F_G) {
		// ↓ trans のモデルハンドル名に合わせて変更してください
		MV1SetOpacityRate(trans.modelId, 0.65f);
		MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_ALWAYS);
	}

	// ガラスは当たり判定に入れない
	if (data.modelSrc != ResourceManager::SRC::F_G) {
		player_->AddFurniture(f);
	}

	// 描画リストを分ける
	if (data.modelSrc == ResourceManager::SRC::F_G) {
		glassFurnitures_.push_back(f);
	}
	else {
		furnitures_.push_back(f);
	}

}
