#pragma once

#include <map>
#include <vector>
#include <set>
#include <DxLib.h>

#include "../../Manager/ResourceManager.h"
#include "../../Utility/AsoUtility.h"
#include "../../Common/Vector3.h"
#include "../Common/Transform.h"
#include "../Furniture/Furniture.h"
#include "../Furniture/StoneDevice.h"
#include "../Item.h"

class ResourceManager;
class WarpStar;
class Planet;
class Player;

class Stage
{
public:

	// ステージの切り替え間隔
	static constexpr float TIME_STAGE_CHANGE = 1.0f;

	// 家具の設計図
	struct FurnitureData
	{
		ResourceManager::SRC modelSrc;
		Vector3 pos;
		Vector3 scl;
		Vector3 rot;
	};

	Stage(Player* player);
	~Stage(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	// ステージ変更
	void ChangeStage(NAME type);

	// 対象ステージを取得
	Planet* GetPlanet(NAME type);

	void CreateFurniture(const FurnitureData& data);

private:

	// シングルトン参照
	ResourceManager& resMng_;

	Player* player_;
	StoneDevice* stoneDevice_;

	// ステージアクティブになっている惑星の情報
	NAME activeName_;
	Planet* activePlanet_;

	// 惑星
	std::map<NAME, Planet*> stages_;

	// 家具
	std::vector<Furniture*> furnitures_;
	std::vector<Furniture*> glassFurnitures_;

	// ワープスター
	std::vector<WarpStar*> warpStars_;

	// 空のPlanet
	Planet* nullPlanet = nullptr;

	float step_;

	// 最初の惑星
	void MakeMainStage(void);

	// ワープスター
	void MakeWarpStar(void);

	void CreateFirstStage(void);

	// アイテム
	std::vector<Item*> items_;

	int itemCount_[(int)Item::TYPE::MAX];
	int registeredItemCount_[(int)Item::TYPE::MAX];
	int stolenItemCount_[(int)Item::TYPE::MAX];

	// 所持上限：3個のノートPCを拾えるようにする
	static constexpr int MAX_ITEM_COUNT = 3;

	int lookingItemIndex_;
	bool isItemMax_;

	// アイテム出現候補地点
	std::vector<VECTOR> itemSpawnPoints_;

	void CreateItem(
		Item::TYPE type,
		ResourceManager::SRC modelSrc,
		VECTOR pos,
		VECTOR scl);

	// 候補地点からノートPCをランダム生成
	void CreateRandomLaptopItemsFromSpawnPoints(int count);

	int FindLookingItem(void);
	void UpdateItemPickup(void);

	bool HasAnyItem(void) const;
	void RegisterItemsToStoneDevice(void);
	void UpdateStoneDeviceRegister(void);

	void DrawItemUI(void) const;

	int GetTotalItemCount(void) const;

	int GetItemPrice(Item::TYPE type) const;
	int CalcStolenMoney(void) const;
	int CalcTotalMoney(void) const;
	int CalcRemainDay(void) const;
};
