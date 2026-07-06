#include <vector>
#include <map>
#include <DxLib.h>
#include <set>
#include <assert.h>

#include "../../Utility/AsoUtility.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/InputManager.h"
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
#include "../Furniture/BookShelf.h"
#include "../Furniture/Locker.h"
#include "../Furniture/Freezer.h"
#include "../Furniture/Book.h"
#include "../Furniture/Door.h"
#include "../Furniture/Trashcan.h"
#include "../Furniture/Button.h"
#include "../../Shader/Light/LightManager.h"
#include "../../Shader/Light/LightEffect.h"
#include "../../Shader/RimLightEffect.h"

Stage::Stage(Player* player)
	: resMng_(ResourceManager::GetInstance())
{
	player_ = player;
	activeName_ = NAME::FIRST_STAGE;
	activePlanet_ = nullptr;
	step_ = 0.0f;
	stoneDevice_ = nullptr;

	// 強制脱出ボタン
	escapeButton_ = nullptr;
	escapeButtonPos_ = VGet(0.0f, 0.0f, 0.0f);
	isEscapeButtonActivated_ = false;

	lookingItemIndex_ = -1;
	isItemMax_ = false;

	miniMapScreen_ = -1;
	isMiniMapVisible_ = false;

	escapeTimeLimit_ = 300 * 1000;
	emergencyEscapeTime_ = 60 * 1000;
	escapeStartTime_ = 0;

	isEscapeTimerStarted_ = false;
	isEmergencyEscape_ = false;
	isResultChanged_ = false;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		itemCount_[i] = 0;
		registeredItemCount_[i] = 0;
		stolenItemCount_[i] = 0;
	}
}
Stage::~Stage(void)
{
	printf("[Stage Destructor] called\n");
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

	// ミニマップ
	if (miniMapScreen_ != -1)
	{
		DeleteGraph(miniMapScreen_);
		miniMapScreen_ = -1;
	}


	ceilingLights_.clear();
	ReleasePostOutline();
	lightEffect_.Release();
	//rimLightEffect_.Release();

	escapeButton_ = nullptr;
}

void Stage::Init(void)
{
	SRand(GetNowCount());

	// HLSLライト用
	bool isLightEffectOk = lightEffect_.Init(
		"Data/Shader/Light3DVS.cso",
		"Data/Shader/Light3DPS.cso"
	);
	//rimLightEffect_.Init(
	//	"Data/Shader/ItemRimLightPS.cso"
	//);


	// ポストエフェクト輪郭線用
	InitPostOutline();

	CreateBeamGraph();

	//	ミニマップ
	miniMapScreen_ = MakeScreen(512, 512, TRUE);

	// ステージ
	MakeMainStage();

	stoneDevice_ = new StoneDevice(player_);
	stoneDevice_->Init();

	step_ = -1.0f;


	// 導入演出が終わるまでタイマーは開始しない
	escapeStartTime_ = 0;
	isEscapeTimerStarted_ = false;
	isEmergencyEscape_ = false;
	isResultChanged_ = false;



}

void Stage::Update(void)
{
	// 惑星
	for (const auto& s : stages_)
	{
		s.second->Update();
	}

	// 家具
    // ゴミ箱隠れ状態を毎フレームリセット
	if (player_ != nullptr)
	{
		player_->SetHiddenInTrashcan(false);
	}

	for (auto furniture : furnitures_)
	{
		Door* door = dynamic_cast<Door*>(furniture);

		if (door != nullptr)
		{
			door->Update(*player_);
			continue;
		}

		Trashcan* trashcan = dynamic_cast<Trashcan*>(furniture);

		if (trashcan != nullptr)
		{
			trashcan->Update(*player_);
			continue;
		}

		furniture->Update();
	}

	// 家具判定が終わった後に、ゴミ箱隠れ状態を反映する
	if (player_ != nullptr)
	{
		player_->ApplyTrashcanHideState();
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
	UpdateEscapeButton();


	if (isEscapeButtonActivated_)
	{
		return;
	}



	// 追加：TABを押している間ミニマップ表示
	isMiniMapVisible_ = CheckHitKey(KEY_INPUT_TAB);


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

	// =========================
	// 脱出時間チェック
	// =========================
	if (!isEscapeTimerStarted_)
	{
		return;
	}

	int nowTime = GetNowCount();
	int elapsedTime = nowTime - escapeStartTime_;

	// 5分経過したら予備の強制脱出装置モードへ
	if (elapsedTime >= escapeTimeLimit_)
	{
		isEmergencyEscape_ = true;
	}

	// 5分 + 1分 経過したらリザルトへ
	if (!isResultChanged_ &&
		elapsedTime >= escapeTimeLimit_ + emergencyEscapeTime_)
	{
		isResultChanged_ = true;

		int stolenMoney = CalcTotalMoney();

		SceneManager::GetInstance().SetResultData(stolenMoney);
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);

		return;
	}

}
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

	SetCameraNearFar(10.0f, 7000);
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


#ifdef _DEBUG
	DebugDrawItemPickupCheck();
	DebugDrawPickupRange();

	if (player_ != nullptr)
	{
		for (auto furniture : furnitures_)
		{
			if (furniture == nullptr)
			{
				continue;
			}

			Door* door = dynamic_cast<Door*>(furniture);

			if (door != nullptr)
			{
				door->DebugDrawInteractRange(*player_);
				door->DebugDrawCollision();
				continue;
			}

			Trashcan* trashcan = dynamic_cast<Trashcan*>(furniture);

			if (trashcan != nullptr)
			{
				trashcan->DebugDrawCollision();
				continue;
			}
		}
	}
#endif

}


void Stage::DrawUI(void) const
{
	DrawEscapeTimeUI();

	if (isEmergencyEscape_)
	{
		DrawEmergencyEscapeUI();
	}

	DrawItemUI();
	DrawInventoryUI();
	DrawPickupUI();

	if (player_ == nullptr)
	{
		return;
	}

	for (auto furniture : furnitures_)
	{
		if (furniture == nullptr)
		{
			continue;
		}

		Door* door = dynamic_cast<Door*>(furniture);

		if (door != nullptr)
		{
			if (door->DrawInteractUI(*player_))
			{
				return;
			}

			continue;
		}

		Trashcan* trashcan = dynamic_cast<Trashcan*>(furniture);

		if (trashcan != nullptr)
		{
			if (trashcan->DrawInteractUI(*player_))
			{
				return;
			}

			continue;
		}
	}

	if (isMiniMapVisible_)
	{
		DrawMiniMap();
	}
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
	watchSpawnPoints_.clear();
	bookSpawnPoints_.clear();

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

	// 机----------------------------------------------------------------------------
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

			// 机の上をノートPCの出現候補
			itemSpawnPoints_.push_back(VGet(posX, -23.0f, posZ));
			watchSpawnPoints_.push_back(VGet(posX, -23.0f, posZ)); // 腕時計
		}
	}

	// 厨房の机

	struct TableData {
		VECTOR pos;
		VECTOR scale;
		float rotY;
	};

	TableData tables[] = {
		{{-1790.0f, -100.0f,  390.0f}, {4.0f, 0.6f, 0.8f}, 90.0f},
		{{-2400.0f, -100.0f,  390.0f}, {4.0f, 0.6f, 1.88f}, 90.0f},
		{{-3000.0f, -100.0f,  390.0f}, {4.0f, 0.6f, 0.8f}, 90.0f},
		{{-2400.0f, -100.0f, -600.0f}, {2.0f, 0.6f, 1.88f}, 90.0f},
		{{-2900.0f, -100.0f, -600.0f}, {2.0f, 0.6f, 1.88f}, 90.0f},
		{{-1920.0f, -100.0f, 1180.0f}, {1.2f, 0.5f, 1.0f}, 0.0f}, // 休憩室
		{{-1175.0f, -100.0f, 1320.0f}, {2.0f, 0.5f, 0.6f}, 0.0f}, // 店長室
	};

	for (auto& t : tables)
	{
		CreateFurniture({
			ResourceManager::SRC::F_TABLE,
            Vector3(t.pos.x, t.pos.y, t.pos.z),
	        Vector3(t.scale.x, t.scale.y, t.scale.z),
			Vector3{ 0.0f, AsoUtility::Deg2RadF(t.rotY), 0.0f }
			});
	}

	// 休憩室の机

	//-------------------------------------------


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
		{ 4.0f, 1.0f, 0.5f },
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
		// ②右右の壁（休憩室と更衣室の間の壁）
		{
		ResourceManager::SRC::WALL,
		{ -2060.0f, -100.0f, 1000.0f },
		{ 2.0f, 1.0f, 0.5f },
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
			// 廊下扉の上の部分
			{
			ResourceManager::SRC::WALL,
			{ -3430.0f, 485.0f, 915.0f },
			{ 0.32f, 1.0f, 0.5f },
			{  AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(-90.0f), 0.0f }
			},

			// ④正面左壁（厨房扉の左の壁）
		{
			ResourceManager::SRC::WALL,
			{ -3430.0f, -100.0f, -1084.0f },
			{ 0.62f, 1.0f, 0.5f },
			{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		},
	};

	for (const auto& wallData : wallDatas)
	{
		CreateFurniture(wallData);
	}

	// 本棚
	CreateFurniture({
	ResourceManager::SRC::BOOKSLF,
	{ -1445.0f, -100.0f, 330.0f },
	{ 0.93f, 0.5f, 1.0f },
	{0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		});

	
	// 本棚の本
	// i = 行, j = 列
	std::set<std::pair<int, int>> itemBookSlots =
	{
		{ 1, 3 },   // 2行目 4列目をアイテム本にする
		{ 2, 7 }    // 3行目 8列目をアイテム本にする
	};

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			float bookposZ = -130.0f + (j * 95.0f);
			float bookposY = -70.0f + (i * 35.0f);

			// この場所をアイテム本にするか
			bool isItemBook =
				itemBookSlots.count({ i, j }) > 0;

			if (isItemBook)
			{
				// 飾り本は作らず、拾えるアイテム本を作る
				CreateItem(
					Item::TYPE::BOOK168,
					ResourceManager::SRC::BOOK168,
					VGet(-1435.0f, bookposY, bookposZ),
					VGet(0.1f, 0.1f, 0.1f),
					VGet(0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f)
				);

				continue;
			}

			// 通常の飾り本
			CreateFurniture({
				ResourceManager::SRC::BOOK,
				{ -1435.0f, bookposY, bookposZ },
				{ 2.0f, 0.2f, 0.3f },
				{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
				});
		}
	}


	// ゴミ箱
	CreateFurniture({
	ResourceManager::SRC::TRASHCAN,
	{ -3700.0f, -100.0f, -550.0f},
	{ 1.0f, 0.618f, 1.0f },
	{0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f }
		});

	// ロッカー--------------------------------------------
	for (int i = 0; i < 13; i++)
	{
		float posX = -2680.0f + (i * -58.0f);

		CreateFurniture({
		ResourceManager::SRC::LOCKER,
		{ posX, -100.0f, 1050.0f },
		{ 0.95f, 1.0f, 1.0f },
		{0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f }
		});

		// ロッカー周辺を腕時計の出現候補にする
		watchSpawnPoints_.push_back(VGet(posX, -90.0f, 1070.0f));

	}

	for (int i = 0; i < 20; i++)
	{
		float posX = -2280.0f + (i * -58.0f);

		CreateFurniture({
		ResourceManager::SRC::LOCKER,
		{ posX, -100.0f, 1350.0f },
		{ 0.95f, 1.0f, 1.0f },
		{0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f }
			});


		// ロッカー周辺を腕時計の出現候補にする
		watchSpawnPoints_.push_back(VGet(posX, -90.0f, 1340.0f));

	}
	CreateFurniture({ // 脱出ボタン
	ResourceManager::SRC::BUTTON,
	{ -3480.0f, -96.0f, 100.0f },
	{ 0.5f, 0.5f, 0.5f },
	{ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f },
	1.0f, -1.0f
		});

	// ドア------------------------------------------------------
	CreateFurniture({ // 厨房のドア
	ResourceManager::SRC::DOOR,
	{ -3430.0f, -96.0f, -890.0f },
	{ 1.1f, 1.0f, 1.0f },
	{ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f },
	1.0f, -1.0f
		});
	CreateFurniture({
	ResourceManager::SRC::WALL,
	{ -3430.0f, 485.0f, -890.0f },
	{ 0.32f, 1.0f, 0.5f },
	{ AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(90.0f), 0.0f }
		});
	CreateFurniture({ // 廊下のドア
	ResourceManager::SRC::DOOR,
	{ -3435.0f, -96.0f, 918.0f },
	{ 1.4f, 1.0f, 1.0f },
	{ 0.0f, AsoUtility::Deg2RadF(90.0f), 0.0f },
	1.0f, -1.0f
		});
	CreateFurniture({ // 休憩室のドア
	ResourceManager::SRC::DOOR,
	{ -1566.0f, -96.0f, 1000.0f },
	{ 1.0f, 1.0f, 1.0f },
	{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f },
	-1.0f,-1.0f
		});
	CreateFurniture({
    ResourceManager::SRC::WALL,
    { -1566.0f, 485.0f, 1000.0f },
    { 0.32f, 1.0f, 0.5f },
    { AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(0.0f), 0.0f }
		});
	CreateFurniture({ // 更衣室のドア
	ResourceManager::SRC::DOOR,
	{ -2559.0f, -96.0f, 1000.0f },
	{ 1.1f, 1.0f, 1.0f },
	{ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f },
	-1.0f, -1.0f
		});
	CreateFurniture({
	ResourceManager::SRC::WALL,
	{ -2559.0f, 485.0f, 1000.0f },
	{ 0.32f, 1.0f, 0.5f },
	{ AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(0.0f), 0.0f }
		});

	// 冷凍庫----------------------------------------------------
	CreateFurniture({
		ResourceManager::SRC::FREEZER,
		{ -3415.0f, -96.0f, -295.0f },
		{ 0.054f, 0.057f, 0.12f },
		{ 0.0f, AsoUtility::Deg2RadF(-90.0f), 0.0f }
		});

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

	// アイテムランダム生成
	CreateRandomLaptopItemsFromSpawnPoints(1); // ノートPC
	CreateRandomWatchItemsFromSpawnPoints(3); // 腕時計


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

void Stage::RegisterLightBlockerByArea(
	LightBlocker* blocker,
	const VECTOR& pos
)
{
	if (blocker == nullptr)
	{
		return;
	}

	// 上にある扉上パーツなどはライト遮蔽から外す
	if (pos.y > 300.0f)
	{
		return;
	}

	// 脱出エリア
	if (pos.x <= -3400.0f && pos.x >= -3950.0f &&
		pos.z >= -450.0f && pos.z <= 650.0f)
	{
		escapeBlockers_.push_back(blocker);
		return;
	}

	// 厨房
	if (pos.x <= -1700.0f && pos.x >= -4300.0f &&
		pos.z <= 850.0f)
	{
		kitchenBlockers_.push_back(blocker);
		return;
	}

	// 廊下・休憩室・更衣室・店長室周辺
	if (pos.z >= 830.0f)
	{
		corridorBlockers_.push_back(blocker);

		if (pos.x <= -2200.0f)
		{
			lockerBlockers_.push_back(blocker);
		}

		if (pos.x >= -1600.0f)
		{
			officeBlockers_.push_back(blocker);
		}

		return;
	}

	// それ以外は客席
	hallBlockers_.push_back(blocker);
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
		f = new Showcase(&trans, data.rot.y);
	}
	else if (data.modelSrc == ResourceManager::SRC::F_G)
	{
		f = new Showcase(&trans, data.rot.y);
	}
	else if (data.modelSrc == ResourceManager::SRC::BOOKSLF)
	{
		f = new BookShelf(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::LOCKER)
	{
		f = new Locker(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::FREEZER)
	{
		f = new Freezer(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::BOOK)
	{
		f = new Book(&trans);
	}
	else if (data.modelSrc == ResourceManager::SRC::DOOR)
	{

		f = new Door(
			&trans,data.rot.y,data.doorHingeSide,data.doorOpenSign
		);
	}
	else if (data.modelSrc == ResourceManager::SRC::BUTTON)
	{
		f = new Button(&trans);

		// 強制脱出ボタンとして記録
		escapeButton_ = dynamic_cast<Button*>(f);
		escapeButtonPos_ = VGet(data.pos.x, data.pos.y, data.pos.z);
	}
	else if (data.modelSrc == ResourceManager::SRC::TRASHCAN)
	{
		f = new Trashcan(&trans);
	}

	if (f == nullptr)
	{
		return;
	}

	f->Init();

	// 壁をライト遮蔽用に登録
	// ただし、LightManagerへ全壁登録はしない。
	// Stage側でエリア別に管理する。
	if (data.modelSrc == ResourceManager::SRC::WALL)
	{
		LightBlocker* blocker =
			dynamic_cast<LightBlocker*>(f);

		if (blocker != nullptr)
		{
			VECTOR wallPos = VGet(
				data.pos.x,
				data.pos.y,
				data.pos.z
			);

			RegisterLightBlockerByArea(blocker, wallPos);
		}
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
	VECTOR scl,
	VECTOR rot)
{
	Item* item = new Item();

	item->Init(
		type,
		modelSrc,
		pos,
		scl,
		rot
	);

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

void Stage::CreateRandomWatchItemsFromSpawnPoints(int count)
{
	if (watchSpawnPoints_.empty())
	{
		return;
	}

	if (count > (int)watchSpawnPoints_.size())
	{
		count = (int)watchSpawnPoints_.size();
	}

	for (int i = 0; i < count; i++)
	{
		int randIndex = i + GetRand((int)watchSpawnPoints_.size() - 1 - i);

		VECTOR temp = watchSpawnPoints_[i];
		watchSpawnPoints_[i] = watchSpawnPoints_[randIndex];
		watchSpawnPoints_[randIndex] = temp;

		CreateItem(
			Item::TYPE::WATCH,
			ResourceManager::SRC::WATCH,
			watchSpawnPoints_[i],
			VGet(0.03f, 0.03f, 0.03f)
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

	int nearestIndex = -1;
	float nearestDistance = 9999999.0f;

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

		// まず距離判定
		if (!items_[i]->IsInPlayerView(playerPos, playerForward))
		{
			continue;
		}

		VECTOR itemPos = items_[i]->GetPos();

		// 判定用の線の開始位置と終了位置
		// プレイヤーの少し上から、アイテムの少し上へ線を飛ばす
		VECTOR lineStart = playerPos;
		lineStart.y += 80.0f;

		VECTOR lineEnd = itemPos;
		lineEnd.y += 20.0f;

		// 壁越しなら対象外
		if (IsPickupLineBlocked(lineStart, lineEnd))
		{
			continue;
		}

		// 一番近いアイテムを選ぶ
		VECTOR toItem = VSub(itemPos, playerPos);
		toItem.y = 0.0f;

		float distance = VSize(toItem);

		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestIndex = i;
		}
	}

	return nearestIndex;
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


	int usedSlotCount = GetUsedInventorySlotCount();
	int itemSlotSize = GetItemSlotSize(type);

	if (usedSlotCount + itemSlotSize > MAX_ITEM_COUNT)
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
		"インベントリ：%d / %d",
		GetUsedInventorySlotCount(),
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
		case Item::TYPE::WATCH:
			name = "腕時計";
			break;
		case Item::TYPE::BOOK168:
			name = "I６８同人誌";
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
		case Item::TYPE::WATCH:
			name = "腕時計";
			break;
		case Item::TYPE::BOOK168:
			name = "I６８同人誌";
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
					"Fキーでアイテム転送",
					GetColor(255, 255, 255)
				);
			}
			else
			{
				DrawString(
					20,
					80,
					"転送できるアイテムがありません",
					GetColor(180, 180, 180)
				);
			}
		}
	}

	if (escapeButton_ != nullptr && !isEscapeButtonActivated_)
	{
		DrawString(
			20,
			105,
			"脱出ボタンを殴ると脱出",
			GetColor(255, 220, 80)
		);
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
		return 320000;

	case Item::TYPE::WATCH:
		return 37800;

	case Item::TYPE::BOOK168:
		return 168000;

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


	VECTOR lightPos = VGet(data.pos.x, data.pos.y, data.pos.z);

	const std::vector<LightBlocker*>& blockers =
		GetLightBlockersByLightPos(lightPos);

	lightEffect_.GetLightManager().AddLight(
		lightPos,
		VGet(0.45f, 0.36f, 0.24f),
		VGet(0.0f, -1.0f, 0.0f),
		950.0f,
		DX_PI_F / 8.0f,
		DX_PI_F / 3.5f,
		blockers
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
	VECTOR lightPos = VGet(data.pos.x, data.pos.y, data.pos.z);

	lightEffect_.GetLightManager().AddLight(
		lightPos,
		VGet(0.50f, 0.52f, 0.58f),
		VGet(0.0f, -1.0f, 0.0f),
		800.0f,
		DX_PI_F / 3.2f,
		DX_PI_F / 3.0f,
		kitchenBlockers_
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
	SetCameraNearFar(10.0f, 7000.0f);
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

	

	if (stoneDevice_ != nullptr)
	{
		stoneDevice_->Draw();
	}

	for (auto item : items_)
	{
		if (item == nullptr)
		{
			continue;
		}

		item->Draw();
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

	// =========================
    // アイテム白点滅
    // =========================
	for (auto item : items_)
	{
		if (item == nullptr)
		{
			continue;
		}

		item->DrawWhiteBlink();
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

void Stage::DrawInventoryUI(void) const
{
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	const int slotCount = MAX_ITEM_COUNT;

	const int slotW = 120;
	const int slotH = 60;
	const int slotMargin = 12;

	const int totalW =
		slotW * slotCount + slotMargin * (slotCount - 1);

	const int startX = screenW / 2 - totalW / 2;
	const int startY = screenH - slotH - 25;

	// まず空の4マスを描く
	for (int i = 0; i < slotCount; i++)
	{
		int x = startX + i * (slotW + slotMargin);
		int y = startY;

		DrawBox(
			x,
			y,
			x + slotW,
			y + slotH,
			GetColor(15, 15, 20),
			TRUE
		);

		DrawBox(
			x,
			y,
			x + slotW,
			y + slotH,
			GetColor(100, 100, 100),
			FALSE
		);
	}

	int slotIndex = 0;

	// 所持アイテムを順番に表示
	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		Item::TYPE type = (Item::TYPE)i;

		for (int count = 0; count < itemCount_[i]; count++)
		{
			int slotSize = GetItemSlotSize(type);

			if (slotIndex >= slotCount)
			{
				return;
			}

			if (slotIndex + slotSize > slotCount)
			{
				slotSize = slotCount - slotIndex;
			}

			const char* name = "不明";

			switch (type)
			{
			case Item::TYPE::LAPTOP:
				name = "ノートPC";
				break;

			case Item::TYPE::WATCH:
				name = "腕時計";
				break;

			case Item::TYPE::BOOK168:
				name = "I６８同人誌";
				break;
			case Item::TYPE::MEDICINE:
				name = "薬";
				break;

			default:
				name = "不明";
				break;
			}

			int x = startX + slotIndex * (slotW + slotMargin);
			int y = startY;

			int itemW =
				slotW * slotSize + slotMargin * (slotSize - 1);

			// アイテム背景
			DrawBox(
				x,
				y,
				x + itemW,
				y + slotH,
				GetColor(35, 35, 55),
				TRUE
			);

			// アイテム枠
			DrawBox(
				x,
				y,
				x + itemW,
				y + slotH,
				GetColor(255, 255, 255),
				FALSE
			);

			// アイテム名を中央寄せっぽく表示
			int textX = x + itemW / 2 - 40;
			int textY = y + 20;

			DrawString(
				textX,
				textY,
				name,
				GetColor(255, 255, 255)
			);

			// 何マス使っているか小さく表示
			DrawFormatString(
				x + itemW - 55,
				y + slotH - 20,
				GetColor(180, 180, 180),
				"%dマス",
				slotSize
			);

			slotIndex += slotSize;
		}
	}
}

int Stage::GetItemSlotSize(Item::TYPE type) const
{
	switch (type)
	{
	case Item::TYPE::LAPTOP:
		return 3;

	case Item::TYPE::WATCH:
		return 1;

	case Item::TYPE::BOOK168:
		return 1;

	case Item::TYPE::MEDICINE:
		return 1;

	default:
		return 1;
	}
}

int Stage::GetUsedInventorySlotCount(void) const
{
	int total = 0;

	for (int i = 0; i < (int)Item::TYPE::MAX; i++)
	{
		Item::TYPE type = (Item::TYPE)i;

		total += itemCount_[i] * GetItemSlotSize(type);
	}

	return total;
}

bool Stage::IsPickupLineBlocked(const VECTOR& from, const VECTOR& to) const
{
	for (auto f : furnitures_)
	{
		if (f == nullptr)
		{
			continue;
		}

		// 壁とドアだけを遮蔽物にする
		Wall* wall = dynamic_cast<Wall*>(f);
		Door* door = dynamic_cast<Door*>(f);

		if (wall == nullptr && door == nullptr)
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

void Stage::DrawMiniMap(void) const
{
	if (player_ == nullptr)
	{
		return;
	}

	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	const int mapSize = 460;

	const int mapX = screenW / 2 - mapSize / 2;
	const int mapY = screenH / 2 - mapSize / 2;

	// =========================
	// ワールド座標 → ミニマップ座標
	// =========================
	const float mapCenterX = -2200.0f;
	const float mapCenterZ = 100.0f;
	const float orthoSize = 6000.0f;

	const float scale = (float)mapSize / orthoSize;

	auto WorldToMapX = [&](float worldX) -> int
		{
			return mapX + mapSize / 2 + (int)((worldX - mapCenterX) * scale);
		};

	auto WorldToMapY = [&](float worldZ) -> int
		{
			// +Z方向をマップ上方向にする
			return mapY + mapSize / 2 - (int)((worldZ - mapCenterZ) * scale);
		};

	// =========================
	// 床エリアを描く
	// =========================
	auto DrawGreenRoom = [&](float x1, float z1, float x2, float z2)
		{
			int mx1 = WorldToMapX(x1);
			int my1 = WorldToMapY(z1);
			int mx2 = WorldToMapX(x2);
			int my2 = WorldToMapY(z2);

			int left = mx1;
			int right = mx2;
			int top = my1;
			int bottom = my2;

			if (left > right)
			{
				int temp = left;
				left = right;
				right = temp;
			}

			if (top > bottom)
			{
				int temp = top;
				top = bottom;
				bottom = temp;
			}

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);

			DrawBox(
				left,
				top,
				right,
				bottom,
				GetColor(20, 120, 20),
				TRUE
			);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);

			DrawBox(
				left + 4,
				top + 4,
				right - 4,
				bottom - 4,
				GetColor(70, 255, 70),
				TRUE
			);

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			DrawBox(
				left,
				top,
				right,
				bottom,
				GetColor(120, 255, 80),
				FALSE
			);
		};

	// =========================
	// 脱出エリアを白く半透明で点滅描画
	// =========================
	auto DrawEscapeRoom = [&](float x1, float z1, float x2, float z2)
	{
		int mx1 = WorldToMapX(x1);
		int my1 = WorldToMapY(z1);
		int mx2 = WorldToMapX(x2);
		int my2 = WorldToMapY(z2);

		int left = mx1;
		int right = mx2;
		int top = my1;
		int bottom = my2;

		if (left > right)
		{
			int temp = left;
			left = right;
			right = temp;
		}

		if (top > bottom)
		{
			int temp = top;
			top = bottom;
			bottom = temp;
		}

		// 点滅用
		float t = GetNowCount() / 1000.0f;
		float blink = (sinf(t * 5.0f) + 1.0f) * 0.5f;

		// 透明度を変化させる
		int alpha = 50 + (int)(blink * 100.0f);

		// 白い半透明塗り
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(
			left,
			top,
			right,
			bottom,
			GetColor(255, 255, 255),
			TRUE
		);

		// 内側をさらに薄く光らせる
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha / 2);
		DrawBox(
			left + 5,
			top + 5,
			right - 5,
			bottom - 5,
			GetColor(255, 255, 255),
			TRUE
		);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 枠線
		DrawBox(
			left,
			top,
			right,
			bottom,
			GetColor(255, 255, 255),
			FALSE
		);

	};

	// =========================
	// 自由な線を描く
	// =========================
	auto DrawMapLine = [&](float x1, float z1, float x2, float z2)
		{
			int mx1 = WorldToMapX(x1);
			int my1 = WorldToMapY(z1);
			int mx2 = WorldToMapX(x2);
			int my2 = WorldToMapY(z2);

			// 壁の影
			DrawLine(
				mx1 + 2,
				my1 + 2,
				mx2 + 2,
				my2 + 2,
				GetColor(0, 50, 0),
				7
			);

			// 壁の発光っぽい外側
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);

			DrawLine(
				mx1,
				my1,
				mx2,
				my2,
				GetColor(80, 255, 80),
				9
			);

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			// 壁本体
			DrawLine(
				mx1,
				my1,
				mx2,
				my2,
				GetColor(200, 255, 120),
				4
			);
		};

	// =========================
	// 横線：X方向に伸びる壁
	// startX : 開始X
	// z      : Z位置
	// length : X方向の長さ
	// =========================
	auto DrawHWall = [&](float startX, float z, float length)
		{
			DrawMapLine(
				startX,
				z,
				startX + length,
				z
			);
		};

	// =========================
	// 縦線：Z方向に伸びる壁
	// x      : X位置
	// startZ : 開始Z
	// length : Z方向の長さ
	// =========================
	auto DrawVWall = [&](float x, float startZ, float length)
		{
			DrawMapLine(
				x,
				startZ,
				x,
				startZ + length
			);
		};

	// =========================
	// マップ全体背景
	// =========================
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 235);

	DrawBox(
		mapX - 18,
		mapY - 42,
		mapX + mapSize + 18,
		mapY + mapSize + 18,
		GetColor(0, 0, 0),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// タイトル背景
	DrawBox(
		mapX - 18,
		mapY - 42,
		mapX + mapSize + 18,
		mapY - 4,
		GetColor(10, 25, 10),
		TRUE
	);

	DrawString(
		mapX,
		mapY - 32,
		"MINI MAP",
		GetColor(180, 255, 180)
	);

	// マップ内背景
	DrawBox(
		mapX,
		mapY,
		mapX + mapSize,
		mapY + mapSize,
		GetColor(5, 20, 5),
		TRUE
	);

	// =========================
	// 床エリア
	// =========================


	DrawGreenRoom(
		65.0f,
		-1170.0f,	
		-4270.0f,
		1370.0f
	);
	// 脱出エリア
	DrawEscapeRoom(
		-3460.0f,
		-110.0f,	
		-3755.0f,
		590.0f
	);

	


	// =========================
	// ここから手動で壁を書く
	// 通れる場所は書かない
	// =========================
	

	// 厨房と客席の間の通路の壁
	// （客席側）
	DrawVWall(-1500.0f, -1140.0f, 770.0f);// 左
	DrawVWall(-1500.0f, -170.0f, 1000.0f);// 右
	// （厨房側）
	DrawVWall(-1700.0f, -1140.0f, 770.0f);// 左
	DrawVWall(-1700.0f, -170.0F, 1000.0F);// 左

	DrawVWall(-3430.0f, -1140.0f, 190.0f);// 左
	DrawVWall(-3430.0f, -850.0f, 440.0f);// 右

	DrawVWall(-3430.0f, -170.0f, 970.0f);
	DrawVWall(-3900.0f, -420.0f, 250.0f);

	DrawVWall(-1400.0f, 830.0f, 270.0f);
	DrawVWall(-300.0f, 830.0f, 590.0f);

	DrawVWall(-2070.0f, 1030.0f, 340.0f);
	DrawVWall(-3430.0f, 1030.0f, 340.0f);

	DrawVWall(-1400.0f, 1280.0f, 90.0f);

	

	// 縦
	DrawHWall(-2450.0f, 1000.0f, 780.0f); // 休憩室と更衣室の間
	DrawHWall(-3910.0f, -420.0f, 510.0f); // 冷凍庫

	DrawHWall(-3910.0f, -170.0f, 510.0f); // 冷凍庫
	DrawHWall(-1530.0f, 1000.0f, 780.0f); // 店長室

	DrawHWall(-590.0f, 1000.0f, 290.0f); // 店長室
	DrawHWall(-3400.0f, 1000.0f, 830.0f); // 更衣室

	DrawHWall(-1440.0f, 800.0f, 1500.0F); // 客室
	DrawHWall(-3430.0f, 830.0f, 1790.0f); // 厨房
	

	// ---- 外周 ----
	DrawHWall(-4300.0f, -1200.0f, 4400.0f);
	DrawVWall(-4300.0f, -1200.0f, 2600.0f);
	DrawHWall(-4300.0f, 1400.0f, 4400.0f);
	DrawVWall(100.0f, -1200.0f, 2600.0f);



	// =========================
	// マップ外枠
	// =========================
	DrawBox(
		mapX,
		mapY,
		mapX + mapSize,
		mapY + mapSize,
		GetColor(120, 255, 80),
		FALSE
	);

	DrawBox(
		mapX + 3,
		mapY + 3,
		mapX + mapSize - 3,
		mapY + mapSize - 3,
		GetColor(40, 120, 40),
		FALSE
	);

	// =========================
	// プレイヤー位置
	// =========================
	VECTOR playerPos = player_->GetPos();

	int px = WorldToMapX(playerPos.x);
	int py = WorldToMapY(playerPos.z);

	if (px < mapX) px = mapX;
	if (px > mapX + mapSize) px = mapX + mapSize;
	if (py < mapY) py = mapY;
	if (py > mapY + mapSize) py = mapY + mapSize;

	float t = GetNowCount() / 1000.0f;
	float blink = (sinf(t * 8.0f) + 1.0f) * 0.5f;

	int alpha = 90 + (int)(blink * 140.0f);
	int radius = 8 + (int)(blink * 6.0f);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	DrawCircle(
		px,
		py,
		radius + 10,
		GetColor(255, 0, 0),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawCircle(
		px,
		py,
		radius,
		GetColor(255, 0, 0),
		TRUE
	);

	DrawCircle(
		px,
		py,
		3,
		GetColor(255, 255, 255),
		TRUE
	);

	DrawCircle(
		px,
		py,
		radius,
		GetColor(255, 255, 255),
		FALSE
	);

	// プレイヤーの向き
	VECTOR forward = player_->GetForward();
	forward.y = 0.0f;

	if (VSize(forward) > 0.001f)
	{
		forward = VNorm(forward);

		int dirX = (int)(forward.x * 28.0f);
		int dirY = (int)(-forward.z * 28.0f);

		DrawLine(
			px,
			py,
			px + dirX,
			py + dirY,
			GetColor(255, 80, 80),
			3
		);
	}

	// =========================
	// 説明
	// =========================
	int legendY = mapY + mapSize + 8;

	DrawCircle(
		mapX + 12,
		legendY + 8,
		5,
		GetColor(255, 0, 0),
		TRUE
	);

	DrawString(
		mapX + 25,
		legendY,
		"Player",
		GetColor(230, 230, 230)
	);

	DrawLine(
		mapX + 105,
		legendY + 8,
		mapX + 145,
		legendY + 8,
		GetColor(200, 255, 120),
		4
	);

	DrawString(
		mapX + 155,
		legendY,
		"Wall",
		GetColor(230, 230, 230)
	);

	DrawString(
		mapX + mapSize - 150,
		legendY,
		"TABを離すと閉じます",
		GetColor(200, 200, 200)
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage::DebugDrawPickupRange(void) const
{
#ifdef _DEBUG
	if (player_ == nullptr)
	{
		return;
	}

	VECTOR playerPos = player_->GetPos();

	// Item.cpp の pickupRange_ と同じ値
	const float pickupRange = 200.0f;

	// プレイヤーの足元あたりに表示
	VECTOR center = VGet(
		playerPos.x,
		playerPos.y + 5.0f,
		playerPos.z
	);

	const int div = 64;

	for (int i = 0; i < div; i++)
	{
		float a0 = DX_TWO_PI_F * i / div;
		float a1 = DX_TWO_PI_F * (i + 1) / div;

		VECTOR p0 = VGet(
			center.x + cosf(a0) * pickupRange,
			center.y,
			center.z + sinf(a0) * pickupRange
		);

		VECTOR p1 = VGet(
			center.x + cosf(a1) * pickupRange,
			center.y,
			center.z + sinf(a1) * pickupRange
		);

		DrawLine3D(
			p0,
			p1,
			GetColor(0, 255, 0)
		);
	}

	// プレイヤーの向きも描画
	VECTOR forward = player_->GetForward();
	forward.y = 0.0f;

	if (VSize(forward) > 0.001f)
	{
		forward = VNorm(forward);

		VECTOR end = VAdd(
			center,
			VScale(forward, pickupRange)
		);

		DrawLine3D(
			center,
			end,
			GetColor(255, 0, 0)
		);
	}
#endif
}

void Stage::DebugDrawItemPickupCheck(void) const
{
#ifdef _DEBUG
	if (player_ == nullptr)
	{
		return;
	}

	VECTOR playerPos = player_->GetPos();
	VECTOR playerForward = player_->GetForward();

	VECTOR playerGround = playerPos;
	playerGround.y += 10.0f;

	for (int i = 0; i < (int)items_.size(); i++)
	{
		Item* item = items_[i];

		if (item == nullptr)
		{
			continue;
		}

		if (!item->IsActive())
		{
			continue;
		}

		VECTOR itemPos = item->GetPos();

		VECTOR toItem = VSub(itemPos, playerPos);
		toItem.y = 0.0f;

		float distance = VSize(toItem);

		bool inRange = item->IsInPlayerView(playerPos, playerForward);

		int color = inRange
			? GetColor(0, 255, 0)
			: GetColor(255, 0, 0);

		// アイテム判定中心
		DrawSphere3D(
			itemPos,
			20.0f,
			8,
			color,
			color,
			FALSE
		);

		// プレイヤーからアイテム判定中心への線
		VECTOR itemGround = itemPos;
		itemGround.y += 10.0f;

		DrawLine3D(
			playerGround,
			itemGround,
			color
		);

		// 画面上に距離表示
		VECTOR screenPos = ConvWorldPosToScreenPos(itemPos);

		DrawFormatString(
			(int)screenPos.x,
			(int)screenPos.y,
			color,
			"%s Dist: %.1f / %.1f %s",
			item->GetName(),
			distance,
			item->GetPickupRange(),
			inRange ? "OK" : "NG"
		);
	}
#endif
}

void Stage::DrawEscapeTimeUI(void) const
{
	if (!isEscapeTimerStarted_)
	{
		return;
	}

	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);


	int nowTime = GetNowCount();
	int elapsedTime = nowTime - escapeStartTime_;

	int remainTime = 0;

	if (!isEmergencyEscape_)
	{
		// 通常の5分カウント
		remainTime = escapeTimeLimit_ - elapsedTime;
	}
	else
	{
		// 5分経過後の1分カウント
		int emergencyElapsedTime = elapsedTime - escapeTimeLimit_;
		remainTime = emergencyEscapeTime_ - emergencyElapsedTime;
	}

	if (remainTime < 0)
	{
		remainTime = 0;
	}

	int remainSec = remainTime / 1000;
	int minute = remainSec / 60;
	int second = remainSec % 60;

	char timeText[128];

	if (!isEmergencyEscape_)
	{
		sprintf_s(timeText, "脱出時間  %02d:%02d", minute, second);
	}
	else
	{
		sprintf_s(timeText, "強制脱出装置 起動まで  %02d:%02d", minute, second);
	}

	int textW = GetDrawStringWidth(timeText, strlen(timeText));

	int x = screenW / 2 - textW / 2;
	int y = 20;

	int textColor = GetColor(255, 255, 255);

	if (isEmergencyEscape_)
	{
		textColor = GetColor(255, 80, 80);
	}
	else if (remainSec <= 30)
	{
		textColor = GetColor(255, 80, 80);
	}
	else if (remainSec <= 60)
	{
		textColor = GetColor(255, 220, 80);
	}

	// 背景
	DrawBox(
		x - 25,
		y - 10,
		x + textW + 25,
		y + 45,
		GetColor(0, 0, 0),
		TRUE
	);

	// 枠
	DrawBox(
		x - 25,
		y - 10,
		x + textW + 25,
		y + 45,
		textColor,
		FALSE
	);

	// 文字
	DrawString(
		x,
		y,
		timeText,
		textColor
	);
}

void Stage::DrawEmergencyEscapeUI(void) const
{
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	const char* message1 = "5分経過致しました。";
	const char* message2 = "脱出装置が停止されました。";
	const char* message3 = "予備の強制脱出装置を作動させます。";
	const char* message4 = "起動まであと1分掛かります。";
	const char* message5 = "それまで耐えてください。";

	int boxW = 720;
	int boxH = 190;

	int boxX = screenW / 2 - boxW / 2;
	int boxY = 90;

	// 点滅用
	float t = GetNowCount() / 1000.0f;
	float blink = (sinf(t * 6.0f) + 1.0f) * 0.5f;

	int borderColor = GetColor(
		255,
		80 + (int)(blink * 100.0f),
		80
	);

	// 半透明背景
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);

	DrawBox(
		boxX,
		boxY,
		boxX + boxW,
		boxY + boxH,
		GetColor(20, 0, 0),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 枠
	DrawBox(
		boxX,
		boxY,
		boxX + boxW,
		boxY + boxH,
		borderColor,
		FALSE
	);

	DrawBox(
		boxX + 4,
		boxY + 4,
		boxX + boxW - 4,
		boxY + boxH - 4,
		GetColor(120, 30, 30),
		FALSE
	);

	// タイトル
	const char* title = "WARNING";
	int titleW = GetDrawStringWidth(title, strlen(title));

	DrawString(
		screenW / 2 - titleW / 2,
		boxY + 15,
		title,
		GetColor(255, 80, 80)
	);

	// メッセージ中央揃え
	const char* messages[] =
	{
		message1,
		message2,
		message3,
		message4,
		message5
	};

	int startY = boxY + 50;

	for (int i = 0; i < 5; i++)
	{
		int textW = GetDrawStringWidth(messages[i], strlen(messages[i]));

		DrawString(
			screenW / 2 - textW / 2,
			startY + i * 24,
			messages[i],
			GetColor(255, 255, 255)
		);
	}
}

void Stage::StartEscapeTimer(void)
{
	escapeStartTime_ = GetNowCount();

	isEscapeTimerStarted_ = true;
	isEmergencyEscape_ = false;
	isResultChanged_ = false;
}

void Stage::UpdateEscapeButton()
{
	if (player_ == nullptr)
	{
		return;
	}

	if (escapeButton_ == nullptr)
	{
		return;
	}

	if (isEscapeButtonActivated_)
	{
		return;
	}

	// 攻撃中でなければ判定しない
	if (!player_->IsAttacking())
	{
		return;
	}

	VECTOR attackPos = player_->GetAttackPos();

	VECTOR toButton = VSub(escapeButtonPos_, attackPos);
	float distance = VSize(toButton);

	// パンチがボタンに当たる距離
	const float HIT_RANGE = 90.0f;

	if (distance <= HIT_RANGE)
	{
		ActivateEscapeButton();
	}
}

void Stage::ActivateEscapeButton()
{
	if (isEscapeButtonActivated_)
	{
		return;
	}

	isEscapeButtonActivated_ = true;
	isResultChanged_ = true;

	// 持っているアイテム + StoneDeviceに転送済みのアイテムを換金する
	int stolenMoney = CalcTotalMoney();

	SceneManager::GetInstance().SetResultData(stolenMoney);
	SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
}

void Stage::DrawPickupUI(void) const
{
	if (lookingItemIndex_ == -1)
	{
		return;
	}

	if (lookingItemIndex_ < 0 || lookingItemIndex_ >= (int)items_.size())
	{
		return;
	}

	Item* item = items_[lookingItemIndex_];

	if (item == nullptr)
	{
		return;
	}

	if (!item->IsActive())
	{
		return;
	}

	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);

	Item::TYPE type = item->GetType();

	const char* itemName = item->GetName();
	int price = GetItemPrice(type);
	int slotSize = GetItemSlotSize(type);

	// UI位置
	const int boxW = 520;
	const int boxH = 110;

	const int boxX = screenW / 2 - boxW / 2;
	const int boxY = screenH - 230;

	// 点滅
	float t = GetNowCount() / 1000.0f;
	float blink = (sinf(t * 6.0f) + 1.0f) * 0.5f;

	int borderAlpha = 140 + (int)(blink * 100.0f);

	// 持てるかどうか
	bool canPickup = !isItemMax_;

	int mainColor = canPickup
		? GetColor(255, 255, 255)
		: GetColor(255, 80, 80);

	int accentColor = canPickup
		? GetColor(120, 220, 255)
		: GetColor(255, 80, 80);

	// 背景
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);

	DrawBox(
		boxX,
		boxY,
		boxX + boxW,
		boxY + boxH,
		GetColor(10, 15, 25),
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 枠
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, borderAlpha);

	DrawBox(
		boxX,
		boxY,
		boxX + boxW,
		boxY + boxH,
		accentColor,
		FALSE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// アイテム名
	DrawFormatString(
		boxX + 25,
		boxY + 18,
		mainColor,
		"%s",
		itemName
	);

	// 価値・枠数
	DrawFormatString(
		boxX + 25,
		boxY + 48,
		GetColor(210, 210, 210),
		"必要枠：%d",
		slotSize
	);

	// Fキー枠
	int keyBoxX = boxX + boxW - 120;
	int keyBoxY = boxY + 28;

	DrawBox(
		keyBoxX,
		keyBoxY,
		keyBoxX + 48,
		keyBoxY + 48,
		canPickup ? GetColor(255, 255, 255) : GetColor(120, 60, 60),
		FALSE
	);

	DrawString(
		keyBoxX + 17,
		keyBoxY + 14,
		"F",
		mainColor
	);

	if (canPickup)
	{
		DrawString(
			keyBoxX + 60,
			keyBoxY + 15,
			"拾う",
			GetColor(255, 255, 255)
		);
	}
	else
	{
		DrawString(
			boxX + 25,
			boxY + 78,
			"インベントリがいっぱいです",
			GetColor(255, 80, 80)
		);
	}
}

const std::vector<LightBlocker*>& Stage::GetLightBlockersByLightPos(
	const VECTOR& lightPos
) const
{
	// 脱出エリア
	if (lightPos.x <= -3400.0f && lightPos.x >= -3950.0f &&
		lightPos.z >= -450.0f && lightPos.z <= 650.0f)
	{
		return escapeBlockers_;
	}

	// 厨房
	if (lightPos.x <= -1700.0f && lightPos.x >= -4300.0f &&
		lightPos.z <= 850.0f)
	{
		return kitchenBlockers_;
	}

	// 廊下・更衣室・休憩室・店長室
	if (lightPos.z >= 830.0f)
	{
		return corridorBlockers_;
	}

	// 客席
	return hallBlockers_;
}