#include <vector>
#include <map>
#include <DxLib.h>
#include <set>
#include <assert.h>

#include "../../Utility/AsoUtility.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
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
#include "../Furniture/StoneDevice.h"
#include "../../Shader/Light/LightManager.h"
#include "../../Shader/Light/LightEffect.h"

Stage::Stage(Player* player)
	: resMng_(ResourceManager::GetInstance())
{
	player_ = player;
	activeName_ = NAME::FIRST_STAGE;
	activePlanet_ = nullptr;
	step_ = 0.0f;
	stoneDevice_ = nullptr;

	lookingItemIndex_ = -1;
	isItemMax_ = false;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		itemCount_[i] = 0;
		registeredItemCount_[i] = 0;
		stolenItemCount_[i] = 0;
	}
}

Stage::~Stage(void)
{
	// 惑星
	for (auto pair : stages_)
	{
		delete pair.second;
	}
	stages_.clear();

	// 家具
	for (auto f : furnitures_)
	{
		delete f;
	}
	furnitures_.clear();

	for (auto f : glassFurnitures_)
	{
		delete f;
	}
	glassFurnitures_.clear();

	if (stoneDevice_ != nullptr)
	{
		delete stoneDevice_;
		stoneDevice_ = nullptr;
	}

	for (auto item : items_)
	{
		delete item;
	}
	items_.clear();

	for (auto l : ceilingLights_)
	{
		delete l;
	}

	ceilingLights_.clear();
	ReleasePostOutline();
	lightEffect_.Release();
}

void Stage::Init(void)
{
	SRand(GetNowCount());

	// HLSLライト用
	bool isLightEffectOk = lightEffect_.Init(
		"Data/Shader/Light3DVS.cso",
		"Data/Shader/Light3DPS.cso"
	);

	// ポストエフェクト輪郭線用
	InitPostOutline();

	CreateBeamGraph();
	// ステージ
	MakeMainStage();

	stoneDevice_ = new StoneDevice(player_);
	stoneDevice_->Init();

	step_ = -1.0f;
}

void Stage::Update(void)
{
	// 惑星
	for (const auto& s : stages_)
	{
		s.second->Update();
	}

	// 家具
	for (auto f : furnitures_)
	{
		f->Update();
	}

	for (auto f : glassFurnitures_)
	{
		f->Update();
	}

	if (stoneDevice_ != nullptr)
	{
		stoneDevice_->Update();
	}

	for (auto item : items_)
	{
		item->Update();
	}
	for (auto l : ceilingLights_)
	{
		l->Update();
	}

	UpdateItemPickup();
	UpdateStoneDeviceRegister();

	if (player_ != nullptr)
	{
		lightEffect_.GetLightManager().SetViewPoint(
			player_->GetPos()
		);

		if (player_->IsFlashLightOn())
		{
			lightEffect_.GetLightManager().SetFlashLight(
				player_->GetFlashLightPos(),
				VGet(0.95f, 0.90f, 0.70f),
				player_->GetFlashLightDir(),
				1200.0f,
				DX_PI_F / 16.0f,
				DX_PI_F / 5.0f
			);
		}
		else
		{
			lightEffect_.GetLightManager().DisableFlashLight();
		}
	}


}
//void Stage::Draw(void)
//{
//	VECTOR cameraPos = GetCameraPosition();
//	VECTOR cameraTarget = GetCameraTarget();
//
//	// 重要：描画直前の正しいカメラ方向でライトを更新
//	UpdateFlashLightForShader(cameraPos, cameraTarget);
//
//	// RTに不透明物 + 半透明物 + 光を描く
//	DrawOpaqueSceneForOutline(cameraPos, cameraTarget);
//
//	// 完成したRTを画面に出す
//	DrawPostOutline();
//
//
//	// =========================
//	 // Stage外の3D描画用に状態を戻す
//	 // =========================
//	SetDrawScreen(DX_SCREEN_BACK);
//
//	SetCameraNearFar(1.0f, 10000.0f);
//	SetupCamera_Perspective(DX_PI_F / 3.0f);
//	SetCameraPositionAndTarget_UpVecY(cameraPos, cameraTarget);
//
//	SetUseZBuffer3D(TRUE);
//	SetWriteZBuffer3D(TRUE);
//	SetUseBackCulling(TRUE);
//	SetUseLighting(TRUE);
//	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
//
//	SetUseVertexShader(-1);
//	SetUsePixelShader(-1);
//
//	SetUseTextureToShader(0, -1);
//	SetUseTextureToShader(1, -1);
//	SetUseTextureToShader(2, -1);
//
//
//	// UIだけ最後
//	DrawItemUI();
//}
void Stage::Draw(void)
{
	// SceneManager が設定している描画先を保存する
	// 通常は mainScrenn_ が入っている
	int oldScreen = GetDrawScreen();

	VECTOR cameraPos = GetCameraPosition();
	VECTOR cameraTarget = GetCameraTarget();

	// 描画直前の正しいカメラ方向でライトを更新
	UpdateFlashLightForShader(cameraPos, cameraTarget);

	// ステージ専用RTに、
	// 不透明物、半透明物、ライト、輪郭線用情報を描画
	DrawOpaqueSceneForOutline(cameraPos, cameraTarget);

	// 完成したアウトライン付きステージ画像を、
	// 元の描画先に描く
	DrawPostOutline(oldScreen);

	// =========================
	// Stage外の3D描画用に状態を戻す
	// =========================

	SetDrawScreen(oldScreen);

	SetCameraNearFar(1.0f, 10000.0f);
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraPositionAndTarget_UpVecY(cameraPos, cameraTarget);

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseBackCulling(TRUE);
	SetUseLighting(TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetUseVertexShader(-1);
	SetUsePixelShader(-1);

	SetUseTextureToShader(0, -1);
	SetUseTextureToShader(1, -1);
	SetUseTextureToShader(2, -1);

	// UIだけ最後
	DrawItemUI();
}
void Stage::ChangeStage(NAME type)
{
	activeName_ = type;

	activePlanet_ = GetPlanet(activeName_);

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
	// アイテム候補地点を初期化
	itemSpawnPoints_.clear();

	// 最初のステージ
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::FLOOR));
	planetTrans.scl = { 10.0f, 10.0f, 10.0f };
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { 0.0f, -100.0f, 0.0f };

	planetTrans.MakeCollider(Collider::TYPE::STAGE);
	planetTrans.Update();

	NAME name = NAME::FIRST_STAGE;
	Planet* stage = new Planet(name, Planet::TYPE::GROUND, planetTrans);
	stage->Init();
	stages_.emplace(name, stage);
	//------------------------------------------------------------------------------

	std::set<int> skipX = { 1, 2 };
	std::set<int> skipZ = { 2, 3, 6, 7 };

	for (int i = 0; i < 4; i++)
	{
		if (skipX.count(i) > 0)
		{
			continue;
		}

		for (int j = 0; j < 10; j++)
		{
			if (skipZ.count(j) > 0)
			{
				continue;
			}

			float posZ = 5.0f + (j * 75.0f);
			float posX = 5.0f + (i * -120.0f);

			CreateFurniture({
				ResourceManager::SRC::F_TABLE,
				{ posX, -100.0f, posZ },
				{ 0.5f, 0.5f, 0.5f },
				{ 0.0f, 0.0f, 0.0f }
				});

			// 机の上をノートPCの出現候補にする
			// 高さが合わない場合は -55.0f を調整してください
			itemSpawnPoints_.push_back(VGet(posX, -23.0f, posZ));
		}
	}


	CreateFurniture({
		ResourceManager::SRC::F_TABLE,
		{ -1829.0f, -100.0f, 283.0f },
		{ 0.5f, 0.5f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f }
		});



	// 壁一覧
	std::vector<FurnitureData> wallDatas =
	{
		// 正面壁
		{
			ResourceManager::SRC::WALL,
			{ 88.0f, -100.0f, -290.0f },
			{ 5.0f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f }
		},
		// ②正面左壁
		{
			ResourceManager::SRC::WALL,
			{ -1500.0f, -100.0f, -805.0f },
			{ 1.8f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},

		// ②正面右壁
		// -x 奥に進
		// -z 左に進
		{
			ResourceManager::SRC::WALL,
			{ -1500.0f, -100.0f, 330.0f },
			{ 2.2f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},

		// ③正面右壁
		{
			ResourceManager::SRC::WALL,
			{ -1700.0f, -100.0f, 330.0f },
			{ 2.2f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
		// ③正面左壁（食器受け取り場所の右の壁）
	{
		ResourceManager::SRC::WALL,
		{ -1700.0f, -100.0f, -610.0f },
		{ 0.9f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
	},
		// ③正面左壁（食器受け取り場所）
	{
		ResourceManager::SRC::WALL,
		{ -1700.0f, -330.0f, -1020.0f },
		{ 0.97f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
	},
		

		// 正面壁（着替え室の壁）
		{
			ResourceManager::SRC::WALL,
			{ -3430.0f, -100.0f, 1200.0f },
			{ 0.9f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
		// 正面壁（休憩室の壁）
		{
			ResourceManager::SRC::WALL,
			{ -2100.0f, -100.0f, 1200.0f },
			{ 0.9f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
		// 正面壁（パソコン室の左壁）
		{
			ResourceManager::SRC::WALL,
			{ -1400.0f, -100.0f, 1350.0f },
			{ 0.25f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
		// 正面壁（パソコン室の右壁）
		{
			ResourceManager::SRC::WALL,
			{ -1400.0f, -100.0f, 950.0f },
			{ 0.65f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
		// 正面壁（パソコン室の後ろ壁）
		{
			ResourceManager::SRC::WALL,
			{ -270.0f, -100.0f, 1200.0f },
			{ 1.9f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
	
		
		// ⑤一番後ろ右の壁
	{
		ResourceManager::SRC::WALL,
		{ -4300.0f, -100.0f, 370.0f },
		{ 5.5f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
	},
		// ⑤正面左壁（厨房扉の左の壁）
	{
		ResourceManager::SRC::WALL,
		{ -4300.0f, -100.0f, -1120.0f },
		{ 0.45f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
	},


		// 左の壁
		{
			ResourceManager::SRC::WALL,
			{ -1000.0f, -100.0f, -1200.0f },
			{ 15.0f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},
		// 右の壁
		{
			ResourceManager::SRC::WALL,
			{ -680.0f, -100.0f, 800.0f },
			{ 3.7f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f }
		},
		// ②右の壁(厨房の右壁）
		{
		ResourceManager::SRC::WALL,
		{ -2565.0f, -100.0f, 829.0f },
		{ 3.87f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},



		// 右右の壁（秘密の部屋の左壁）
		{
		ResourceManager::SRC::WALL,
		{ -400.0f, -100.0f, 1000.0f },
		{ 0.8f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},
		// 右右の壁（秘密の部屋の右壁）
		{
		ResourceManager::SRC::WALL,
		{ -1140.0f, -100.0f, 1000.0f },
		{ 1.7f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},
		// ②右右の壁
		{
		ResourceManager::SRC::WALL,
		{ -2060.0f, -100.0f, 1000.0f },
		{ 1.7f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},
		// ③右右の壁
		{
		ResourceManager::SRC::WALL,
		{ -3030.0f, -100.0f, 1000.0f },
		{ 1.88f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},


		// 一番右の壁
		{
		ResourceManager::SRC::WALL,
		{ -1500.0f, -100.0f, 1400.0f },
		{ 15.0f, 1.0f, 0.5f },
		{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},

		// 冷蔵庫、冷凍庫関連の壁
		//右
		{
			ResourceManager::SRC::WALL,
			{ -3668.5f, -100.0f, -140.0f },
			{ 1.15f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},
		// 左
		{
			ResourceManager::SRC::WALL,
			{ -3668.5f, -100.0f, -450.0f },
			{ 1.15f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
		},
		// 後ろ
		{
			ResourceManager::SRC::WALL,
			{ -3937.0f, -100.0f, -295.0f },
			{ 0.775f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f }
		},
		// ④正面左壁（厨房の冷蔵庫の左の壁）
		{
			ResourceManager::SRC::WALL,
			{ -3430.0f, -100.0f, -633.0f },
			{ 0.9f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},

		// ④正面右壁（厨房の冷蔵庫の右の壁）
		{
			ResourceManager::SRC::WALL,
			{ -3430.0f, -100.0f, 329.0f },
			{ 2.2f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},

			// ④正面左壁（厨房扉の左の壁）
		{
			ResourceManager::SRC::WALL,
			{ -3430.0f, -100.0f, -1092.0f },
			{ 0.43f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
	};

	for (const auto& wallData : wallDatas)
	{
		CreateFurniture(wallData);
	}

	// 天井
	CreateFurniture({
		ResourceManager::SRC::FLOOR,
		{ 10.0f, 250.0f, 10.0f },
		{ 100.0f, 0.5f, 100.0f },
		{ AsoUtility::Deg2RadF(180.0f), 0.0f, 0.0f }
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

	// ノートPCを候補地点から10個ランダム生成
	CreateRandomLaptopItemsFromSpawnPoints(3);


	// 天井ライト
	std::vector<FurnitureData> lightDatas =
	{

	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1370.0f, 250.0f, -1050.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1370.0f, 250.0f, -450.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1370.0f, 250.0f, 0.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1370.0f, 250.0f, 770.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},

		// 廊下
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1610.0f, 250.0f, -775.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1610.0f, 250.0f, -15.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1610.0f, 250.0f, 435.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
		// 廊下出口
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -3200.0f, 250.0f,  900.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -2700.0f, 250.0f,  900.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -2200.0f, 250.0f,  900.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	},
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1700.0f, 250.0f,  900.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	}


	};



	std::vector<FurnitureData> kitchenLightDatas =
	{
	{
		ResourceManager::SRC::CEILING_LIGHT,
		{ -1700.0f, 250.0f,  1200.0f },
		{ 0.1f, 0.08f, 0.1f },
		{ 0.0f, 0.0f, 0.0f }
	}
	};

	// 厨房
	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			FurnitureData data =
			{
				ResourceManager::SRC::CEILING_LIGHT,
				{ -1780 - (j * 523.0f), 250.0f, -1100.0f + (i * 600.0f)},
				{ 0.1f, 0.08f, 0.1f },
				{ 0.0f, 0.0f, 0.0f }
			};
			kitchenLightDatas.push_back(data);
		};

	};


	for (const auto& lightData : kitchenLightDatas)
	{
		CreateKitchenLight(lightData);
	}



	for (const auto& lightData : lightDatas)
	{
		CreateCeilingLight(lightData);
	}
}

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

	trans.Update();

	Furniture* f = nullptr;

	if (data.modelSrc == ResourceManager::SRC::F_TABLE)
	{
		f = new Table(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::WALL)
	{
		f = new Wall(&trans, data.rot.y);
	}
	else if (data.modelSrc == ResourceManager::SRC::FLOOR)
	{
		f = new Ceiling(&trans, data.rot.y);
	}
	else if (data.modelSrc == ResourceManager::SRC::F_F)
	{
		f = new Showcase(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::F_G)
	{
		f = new Showcase(&trans);
	}

	if (f == nullptr)
	{
		return;
	}

	f->Init();

	// 壁をライト遮蔽用に自動登録

	LightBlocker* blocker =
		dynamic_cast<LightBlocker*>(f);

	if (blocker != nullptr)
	{
		lightEffect_
			.GetLightManager()
			.AddWallBlocker(blocker);
	}


	if (data.modelSrc == ResourceManager::SRC::F_G)
	{
		MV1SetOpacityRate(trans.modelId, 0.65f);
		MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_ALWAYS);
	}

	if (data.modelSrc != ResourceManager::SRC::F_G)
	{
		player_->AddFurniture(f);
	}

	if (data.modelSrc == ResourceManager::SRC::F_G)
	{
		glassFurnitures_.push_back(f);
	}
	else
	{
		furnitures_.push_back(f);
	}
}

void Stage::CreateItem(
	Item::TYPE type,
	ResourceManager::SRC modelSrc,
	VECTOR pos,
	VECTOR scl)
{
	Item* item = new Item();
	item->Init(type, modelSrc, pos, scl);
	items_.push_back(item);
}

void Stage::CreateRandomLaptopItemsFromSpawnPoints(int count)
{
	if (itemSpawnPoints_.empty())
	{
		return;
	}

	if (count > (int)itemSpawnPoints_.size())
	{
		count = (int)itemSpawnPoints_.size();
	}

	for (int i = 0; i < count; i++)
	{
		int randIndex = i + GetRand((int)itemSpawnPoints_.size() - 1 - i);

		VECTOR temp = itemSpawnPoints_[i];
		itemSpawnPoints_[i] = itemSpawnPoints_[randIndex];
		itemSpawnPoints_[randIndex] = temp;

		CreateItem(
			Item::TYPE::LAPTOP,
			ResourceManager::SRC::LAPTOP,
			itemSpawnPoints_[i],
			VGet(0.07f, 0.07f, 0.07f)
		);
	}
}

int Stage::FindLookingItem(void)
{
	if (player_ == nullptr)
	{
		return -1;
	}

	VECTOR playerPos = player_->GetPos();
	VECTOR playerForward = player_->GetForward();

	for (int i = 0; i < (int)items_.size(); i++)
	{
		if (items_[i] == nullptr)
		{
			continue;
		}

		if (!items_[i]->IsActive())
		{
			continue;
		}

		if (items_[i]->IsInPlayerView(playerPos, playerForward))
		{
			return i;
		}
	}

	return -1;
}

void Stage::UpdateItemPickup(void)
{
	lookingItemIndex_ = FindLookingItem();
	isItemMax_ = false;

	if (lookingItemIndex_ == -1)
	{
		return;
	}

	auto& ins = InputManager::GetInstance();

	Item* item = items_[lookingItemIndex_];

	Item::TYPE type = item->GetType();
	int typeIndex = (int)type;

	if (GetTotalItemCount() >= MAX_ITEM_COUNT)
	{
		isItemMax_ = true;
		return;
	}

	if (
		ins.IsTrgDown(KEY_INPUT_F) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::LEFT)
		)

	{
		itemCount_[typeIndex]++;

		// 今回のプレイ中に盗んだアイテムとして記録
		stolenItemCount_[typeIndex]++;

		item->Pickup();
	}
}

bool Stage::HasAnyItem(void) const
{
	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		if (itemCount_[i] > 0)
		{
			return true;
		}
	}

	return false;
}

void Stage::RegisterItemsToStoneDevice(void)
{
	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		registeredItemCount_[i] += itemCount_[i];
		itemCount_[i] = 0;
	}
}

void Stage::UpdateStoneDeviceRegister(void)
{
	if (stoneDevice_ == nullptr)
	{
		return;
	}

	if (!stoneDevice_->IsActive())
	{
		return;
	}

	if (!stoneDevice_->IsNearPlayer())
	{
		return;
	}

	auto& ins = InputManager::GetInstance();

	// Eキー：リザルト画面へ
	if (
		ins.IsTrgDown(KEY_INPUT_E) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::TOP)
		)
	{
		int stolenMoney = CalcStolenMoney();

		SceneManager::GetInstance().SetResultData(stolenMoney);
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
		return;
	}

	// アイテムを見ている時はF登録だけ止める
	if (lookingItemIndex_ != -1)
	{
		return;
	}


	if (
		ins.IsTrgDown(KEY_INPUT_F) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::LEFT)
		)
	{
		if (HasAnyItem())
		{
			RegisterItemsToStoneDevice();
		}
	}
}

void Stage::DrawItemUI(void) const
{
	stoneDevice_->DrawUI();
	if (lookingItemIndex_ != -1)
	{
		Item* item = items_[lookingItemIndex_];

		if (item != nullptr)
		{
			if (isItemMax_)
			{
				DrawString(
					20,
					130,
					"これ以上アイテムを持てません",
					GetColor(255, 80, 80)
				);
			}
			else
			{
				DrawFormatString(
					20,
					130,
					GetColor(255, 255, 255),
					"F 拾う：%s",
					item->GetName()
				);
			}
		}
	}

	int x = 20;
	int y = 160;

	DrawFormatString(
		x,
		y,
		GetColor(255, 255, 255),
		"所持アイテム 合計：%d / %d",
		GetTotalItemCount(),
		MAX_ITEM_COUNT
	);

	y += 25;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		const char* name = "不明";

		switch ((Item::TYPE)i)
		{
		case Item::TYPE::LAPTOP:
			name = "ノートPC";
			break;
		case Item::TYPE::BATTERY:
			name = "バッテリー";
			break;
		case Item::TYPE::KEY:
			name = "鍵";
			break;
		case Item::TYPE::MEDICINE:
			name = "薬";
			break;
		default:
			name = "不明";
			break;
		}

		DrawFormatString(
			x,
			y,
			GetColor(255, 255, 255),
			"%s：%d",
			name,
			itemCount_[i]
		);

		y += 20;
	}

	y += 10;

	DrawString(
		x,
		y,
		"登録済みアイテム",
		GetColor(100, 255, 255)
	);

	y += 25;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		const char* name = "不明";

		switch ((Item::TYPE)i)
		{
		case Item::TYPE::LAPTOP:
			name = "ノートPC";
			break;
		case Item::TYPE::BATTERY:
			name = "バッテリー";
			break;
		case Item::TYPE::KEY:
			name = "鍵";
			break;
		case Item::TYPE::MEDICINE:
			name = "薬";
			break;
		default:
			name = "不明";
			break;
		}

		DrawFormatString(
			x,
			y,
			GetColor(100, 255, 255),
			"%s：%d",
			name,
			registeredItemCount_[i]
		);

		y += 20;
	}

	if (stoneDevice_ != nullptr)
	{
		if (stoneDevice_->IsActive() && stoneDevice_->IsNearPlayer())
		{
			if (HasAnyItem())
			{
				DrawString(
					20,
					80,
					"Fキーでアイテム登録 / Eキーで脱出",
					GetColor(255, 255, 255)
				);
			}
			else
			{
				DrawString(
					20,
					80,
					"登録できるアイテムがありません / Eキーで脱出",
					GetColor(180, 180, 180)
				);
			}
		}
	}
}

int Stage::GetTotalItemCount(void) const
{
	int total = 0;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		total += itemCount_[i];
	}

	return total;
}

int Stage::GetItemPrice(Item::TYPE type) const
{
	switch (type)
	{
	case Item::TYPE::LAPTOP:
		// intの上限を超えない値にしています
		// 300億円などを扱いたい場合は、金額系をlong longにしてください
		return 30000;

	case Item::TYPE::BATTERY:
		return 5000;

	case Item::TYPE::KEY:
		return 10000;

	case Item::TYPE::MEDICINE:
		return 3000;

	default:
		return 0;
	}
}

int Stage::CalcStolenMoney(void) const
{
	int total = 0;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		Item::TYPE type = (Item::TYPE)i;
		total += stolenItemCount_[i] * GetItemPrice(type);
	}

	return total;
}

int Stage::CalcTotalMoney(void) const
{
	int total = 0;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		Item::TYPE type = (Item::TYPE)i;
		int count = itemCount_[i] + registeredItemCount_[i];
		total += count * GetItemPrice(type);
	}

	return total;
}

int Stage::CalcRemainDay(void) const
{
	return SceneManager::GetInstance().GetGameRemainDay();
}

void Stage::CreateCeilingLight(const Stage::FurnitureData& data)
{
	Transform trans;

	trans.SetModel(
		resMng_.LoadModelDuplicate(data.modelSrc)
	);

	trans.pos.x = data.pos.x;
	trans.pos.y = data.pos.y;
	trans.pos.z = data.pos.z;

	trans.scl.x = data.scl.x;
	trans.scl.y = data.scl.y;
	trans.scl.z = data.scl.z;

	trans.quaRot =
		Quaternion::Euler(
			data.rot.x,
			data.rot.y,
			data.rot.z
		);

	trans.Update();

	CeilingLight* light =
		new CeilingLight(&trans);

	light->Init();

	ceilingLights_.push_back(light);


	ceilingLightBeamPositions_.push_back(
		VGet(data.pos.x, data.pos.y, data.pos.z)
	);


	lightEffect_.GetLightManager().AddLight(
		VGet(data.pos.x, data.pos.y, data.pos.z),
		VGet(0.45f, 0.36f, 0.24f),
		VGet(0.0f, -1.0f, 0.0f),
		950.0f,
		DX_PI_F / 8.0f,
		DX_PI_F / 3.5f,
		true
	);

}


void Stage::DrawCeilingLightBeams(void)
{
    for (const auto& pos : ceilingLightBeamPositions_)
    {
        DrawOneCeilingLightBeam(pos);
    }
}

void Stage::DrawOneCeilingLightBeam(const VECTOR& lightPos)
{
	if (beamGraph_ == -1)
	{
		return;
	}

	const float topY = lightPos.y - 20.0f;
	const float bottomY = -92.0f;

	const float height = topY - bottomY;

	VECTOR top = VGet(
		lightPos.x,
		topY,
		lightPos.z
	);

	VECTOR bottom = VGet(
		lightPos.x,
		bottomY,
		lightPos.z
	);

	// 光の幅
	float topWidth = 25.0f;
	float bottomWidth = 260.0f;

	// カメラ位置
	VECTOR cameraPos = GetCameraPosition();

	// カメラ方向に向く横方向ベクトルを作る
	VECTOR toCamera = VSub(cameraPos, bottom);
	toCamera.y = 0.0f;

	if (VSize(toCamera) < 0.001f)
	{
		toCamera = VGet(0.0f, 0.0f, 1.0f);
	}

	toCamera = VNorm(toCamera);

	// 横方向
	VECTOR right = VGet(
		toCamera.z,
		0.0f,
		-toCamera.x
	);

	VECTOR topL = VSub(top, VScale(right, topWidth));
	VECTOR topR = VAdd(top, VScale(right, topWidth));

	VECTOR bottomL = VSub(bottom, VScale(right, bottomWidth));
	VECTOR bottomR = VAdd(bottom, VScale(right, bottomWidth));

	VERTEX3D v[6];

	for (int i = 0; i < 6; i++)
	{
		v[i].norm = VGet(0.0f, 1.0f, 0.0f);
		v[i].dif = GetColorU8(255, 255, 255, 255);
		v[i].spc = GetColorU8(0, 0, 0, 0);
	}

	v[0].pos = topL;
	v[0].u = 0.0f;
	v[0].v = 0.0f;

	v[1].pos = bottomL;
	v[1].u = 0.0f;
	v[1].v = 1.0f;

	v[2].pos = bottomR;
	v[2].u = 1.0f;
	v[2].v = 1.0f;

	v[3].pos = topL;
	v[3].u = 0.0f;
	v[3].v = 0.0f;

	v[4].pos = bottomR;
	v[4].u = 1.0f;
	v[4].v = 1.0f;

	v[5].pos = topR;
	v[5].u = 1.0f;
	v[5].v = 0.0f;

	SetUseLighting(FALSE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);

	// 加算合成。ここはかなり薄めでOK
	SetDrawBlendMode(DX_BLENDMODE_ADD, 55);

	DrawPrimitive3D(
		v,
		6,
		DX_PRIMTYPE_TRIANGLELIST,
		beamGraph_,
		TRUE
	);

	// 床の光だまり
	SetDrawBlendMode(DX_BLENDMODE_ADD, 10);


	DrawDisc3D(
		bottom,
		120.0f,
		32,
		GetColor(255, 190, 90)
	);


	SetUseBackCulling(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetUseLighting(TRUE);
}


void Stage::CreateBeamGraph(void)
{
	const int w = 128;
	const int h = 512;

	beamGraph_ = MakeScreen(w, h, TRUE);

	int oldScreen = GetDrawScreen();

	SetDrawScreen(beamGraph_);
	ClearDrawScreen();

	for (int y = 0; y < h; y++)
	{
		float v = (float)y / (float)(h - 1);

		// 上下を薄くする
		float vertical = sinf(v * DX_PI_F);

		for (int x = 0; x < w; x++)
		{
			float u = (float)x / (float)(w - 1);

			// 中央が濃く、左右が薄い
			float center = 1.0f - fabsf(u - 0.5f) * 2.0f;

			if (center < 0.0f)
			{
				center = 0.0f;
			}

			// ふわっとさせる
			center = powf(center, 2.2f);

			float alphaRate = center * vertical;

			int alpha = (int)(alphaRate * 180.0f);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

			DrawPixel(
				x,
				y,
				GetColor(255, 210, 120)
			);
		}
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetDrawScreen(oldScreen);
}

void Stage::DrawDisc3D(
	const VECTOR& center,
	float radius,
	int div,
	int color
)
{
	for (int i = 0; i < div; i++)
	{
		float a0 = DX_TWO_PI_F * i / div;
		float a1 = DX_TWO_PI_F * (i + 1) / div;

		VECTOR p0 = VGet(
			center.x + cosf(a0) * radius,
			center.y,
			center.z + sinf(a0) * radius
		);

		VECTOR p1 = VGet(
			center.x + cosf(a1) * radius,
			center.y,
			center.z + sinf(a1) * radius
		);

		DrawTriangle3D(
			center,
			p0,
			p1,
			color,
			TRUE
		);
	}
}

void Stage::CreateKitchenLight(const Stage::FurnitureData& data)
{
	Transform trans;

	trans.SetModel(
		resMng_.LoadModelDuplicate(data.modelSrc)
	);

	trans.pos.x = data.pos.x;
	trans.pos.y = data.pos.y;
	trans.pos.z = data.pos.z;

	trans.scl.x = data.scl.x;
	trans.scl.y = data.scl.y;
	trans.scl.z = data.scl.z;

	trans.quaRot =
		Quaternion::Euler(
			data.rot.x,
			data.rot.y,
			data.rot.z
		);

	trans.Update();

	CeilingLight* light =
		new CeilingLight(&trans);

	light->Init();

	ceilingLights_.push_back(light);

	// 厨房用ライト
	// 白っぽく、広く、少し強め
	lightEffect_.GetLightManager().AddLight(
		VGet(data.pos.x, data.pos.y, data.pos.z),

		// 色：蛍光灯っぽい白
		VGet(0.50f, 0.52f, 0.58f),

		// 真下
		VGet(0.0f, -1.0f, 0.0f),

		// 届く距離
		800.0f,

		// 中心の明るい範囲
		DX_PI_F / 3.2f,

		// 外側までかなり広く
		DX_PI_F / 3.0f,

		// 壁で遮る
		true
	);

}
bool Stage::InitPostOutline(void)
{
	int w, h;
	GetDrawScreenSize(&w, &h);

	outlineRTColor_ = MakeScreen(w, h, false);
	outlineRTNormal_ = MakeScreen(w, h, false);

	SetCreateDrawValidGraphChannelNum(1);
	SetCreateGraphChannelBitDepth(32);
	outlineRTDepth_ = MakeScreen(w, h, false);
	SetCreateGraphChannelBitDepth(0);
	SetCreateDrawValidGraphChannelNum(4);

	outlinePostPS_ = LoadPixelShader("Data/Shader/PostOutlinePS.cso");

	return true;
}
void Stage::ReleasePostOutline(void)
{
	if (outlineRTColor_ != -1)
	{
		DeleteGraph(outlineRTColor_);
		outlineRTColor_ = -1;
	}

	if (outlineRTNormal_ != -1)
	{
		DeleteGraph(outlineRTNormal_);
		outlineRTNormal_ = -1;
	}

	if (outlineRTDepth_ != -1)
	{
		DeleteGraph(outlineRTDepth_);
		outlineRTDepth_ = -1;
	}

	if (outlinePostPS_ != -1)
	{
		DeleteShader(outlinePostPS_);
		outlinePostPS_ = -1;
	}
}

void Stage::DrawOpaqueSceneForOutline(
	const VECTOR& cameraPos,
	const VECTOR& cameraTarget)
{
	if (outlineRTColor_ == -1 ||
		outlineRTNormal_ == -1 ||
		outlineRTDepth_ == -1)
	{
		return;
	}

	SetDrawScreen(outlineRTColor_);
	ClearDrawScreen();

	// RTに描く時も、保存したプレイヤーカメラを使う
	SetCameraNearFar(1.0f, 10000.0f);
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraPositionAndTarget_UpVecY(cameraPos, cameraTarget);

	FillGraph(outlineRTNormal_, 0, 0, 0, 0);
	FillGraph(outlineRTDepth_, 1.0f, 0, 0, 0);

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetRenderTargetToShader(1, outlineRTNormal_);
	SetRenderTargetToShader(2, outlineRTDepth_);

	lightEffect_.Begin();

	for (const auto& s : stages_)
	{
		s.second->Draw();
	}

	for (auto f : furnitures_)
	{
		if (f == nullptr)
		{
			continue;
		}

		f->Draw();
	}

	for (auto item : items_)
	{
		if (item == nullptr)
		{
			continue;
		}

		item->Draw();
	}

	if (stoneDevice_ != nullptr)
	{
		stoneDevice_->Draw();
	}

	lightEffect_.End();

	// MRT解除
	SetRenderTargetToShader(1, -1);
	SetRenderTargetToShader(2, -1);

	// =================================
	// ここからはRTColorに通常描画する
	// =================================

	// 天井ライト本体
	SetDrawBright(255, 235, 190);

	for (auto l : ceilingLights_)
	{
		if (l != nullptr)
		{
			l->Draw();
		}
	}

	SetDrawBright(255, 255, 255);

	// =========================
	// ガラス家具 半透明描画
	// =========================
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);

	MV1SetSemiTransDrawMode(
		DX_SEMITRANSDRAWMODE_ALWAYS
	);

	SetDrawBlendMode(
		DX_BLENDMODE_ALPHA,
		255
	);

	SetDrawBright(140, 160, 180);

	for (auto f : glassFurnitures_)
	{
		if (f != nullptr)
		{
			f->Draw();
		}
	}

	SetDrawBright(255, 255, 255);

	SetDrawBlendMode(
		DX_BLENDMODE_NOBLEND,
		0
	);

	MV1SetSemiTransDrawMode(
		DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY
	);

	SetUseBackCulling(TRUE);
	SetWriteZBuffer3D(TRUE);

	// =========================
	// 天井ライトの光の柱
	// =========================
	DrawCeilingLightBeams();

	// =========================
	// 天井ライト発光
	// =========================
	for (auto l : ceilingLights_)
	{
		if (l != nullptr)
		{
			l->DrawGlow();
		}
	}

	// =========================
	// RT描画後の状態リセット
	// =========================
	SetRenderTargetToShader(1, -1);
	SetRenderTargetToShader(2, -1);

	SetUseVertexShader(-1);
	SetUsePixelShader(-1);

	SetUseTextureToShader(0, -1);
	SetUseTextureToShader(1, -1);
	SetUseTextureToShader(2, -1);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetUseBackCulling(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseLighting(TRUE);
}

void Stage::DrawPostOutline(int outputScreen)
{
	if (outlineRTColor_ < 0)
	{
		return;
	}

	// 重要：
	// DX_SCREEN_BACKに直接描かない。
	// SceneManagerが指定していた描画先に描く。
	SetDrawScreen(outputScreen);

	if (outlinePostPS_ < 0 ||
		outlineRTNormal_ < 0 ||
		outlineRTDepth_ < 0)
	{
		DrawGraph(0, 0, outlineRTColor_, FALSE);
		return;
	}

	int w, h;
	GetDrawScreenSize(&w, &h);

	// 2Dポリゴン描画用の状態にする
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetUseBackCulling(FALSE);
	SetUseLighting(FALSE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetUseVertexShader(-1);

	SetUseTextureToShader(0, outlineRTColor_);
	SetUseTextureToShader(1, outlineRTNormal_);
	SetUseTextureToShader(2, outlineRTDepth_);

	SetUsePixelShader(outlinePostPS_);

	VERTEX2DSHADER v[6];

	for (int i = 0; i < 6; i++)
	{
		v[i].rhw = 1.0f;
		v[i].dif = GetColorU8(255, 255, 255, 255);
		v[i].spc = GetColorU8(0, 0, 0, 0);
	}

	// 1枚目の三角形
	v[0].pos = VGet(0.0f, 0.0f, 0.0f);
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].su = 0.0f;
	v[0].sv = 0.0f;

	v[1].pos = VGet(0.0f, static_cast<float>(h), 0.0f);
	v[1].u = 0.0f;
	v[1].v = 1.0f;
	v[1].su = 0.0f;
	v[1].sv = 1.0f;

	v[2].pos = VGet(static_cast<float>(w), static_cast<float>(h), 0.0f);
	v[2].u = 1.0f;
	v[2].v = 1.0f;
	v[2].su = 1.0f;
	v[2].sv = 1.0f;

	// 2枚目の三角形
	v[3].pos = VGet(0.0f, 0.0f, 0.0f);
	v[3].u = 0.0f;
	v[3].v = 0.0f;
	v[3].su = 0.0f;
	v[3].sv = 0.0f;

	v[4].pos = VGet(static_cast<float>(w), static_cast<float>(h), 0.0f);
	v[4].u = 1.0f;
	v[4].v = 1.0f;
	v[4].su = 1.0f;
	v[4].sv = 1.0f;

	v[5].pos = VGet(static_cast<float>(w), 0.0f, 0.0f);
	v[5].u = 1.0f;
	v[5].v = 0.0f;
	v[5].su = 1.0f;
	v[5].sv = 0.0f;

	DrawPrimitive2DToShader(
		v,
		6,
		DX_PRIMTYPE_TRIANGLELIST
	);

	SetUseTextureToShader(0, -1);
	SetUseTextureToShader(1, -1);
	SetUseTextureToShader(2, -1);

	SetUsePixelShader(-1);

	// 状態を戻す
	SetUseLighting(TRUE);
	SetUseBackCulling(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

//void Stage::DrawPostOutline(void)
//{
//	if (outlineRTColor_ < 0)
//	{
//		return;
//	}
//
//	SetDrawScreen(DX_SCREEN_BACK);
//
//	// RTをそのまま描画
//	DrawGraph(0, 0, outlineRTColor_, FALSE);
//}
void Stage::UpdateFlashLightForShader(
	const VECTOR& cameraPos,
	const VECTOR& cameraTarget
)
{
	if (player_ == nullptr)
	{
		return;
	}

	// 視点位置もカメラ位置にした方が自然
	lightEffect_.GetLightManager().SetViewPoint(cameraPos);

	if (!player_->IsFlashLightOn())
	{
		lightEffect_.GetLightManager().DisableFlashLight();
		return;
	}

	VECTOR dir = VSub(cameraTarget, cameraPos);

	if (VSize(dir) < 0.001f)
	{
		dir = VGet(0.0f, 0.0f, 1.0f);
	}
	else
	{
		dir = VNorm(dir);
	}

	lightEffect_.GetLightManager().SetFlashLight(
		player_->GetFlashLightPos(),        // ライト位置は手元
		VGet(0.95f, 0.90f, 0.70f),          // 色
		dir,                                // カメラの向き
		1200.0f,
		DX_PI_F / 16.0f,
		DX_PI_F / 5.0f
	);
}

bool Stage::IsLineBlocked(const VECTOR& from, const VECTOR& to) const
{
	for (auto f : furnitures_)
	{
		if (f == nullptr)
		{
			continue;
		}

		int modelId = f->GetModelId();

		MV1_COLL_RESULT_POLY hit =
			MV1CollCheck_Line(
				modelId,
				-1,
				from,
				to
			);

		if (hit.HitFlag)
		{
			return true;
		}
	}

	return false;
}
