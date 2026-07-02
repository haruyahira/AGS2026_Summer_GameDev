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
#include "../Furniture/CeilingLight.h"
#include "../Item.h"
#include "../../Shader/Light/LightManager.h"
#include "../../Shader/Light/LightEffect.h"

class ResourceManager;
class WarpStar;
class Planet;
class Player;
class Wall;

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

public:

    Stage(Player* player);
    ~Stage(void);

    void Init(void);
    void Update(void);
    void Draw(void);

    void DrawUI(void) const;

    // ステージ変更
    void ChangeStage(NAME type);

    // 対象ステージを取得
    Planet* GetPlanet(NAME type);

    void CreateFurniture(const FurnitureData& data);
    void CreateCeilingLight(const FurnitureData& data);
 

    bool IsLineBlocked(const VECTOR& from, const VECTOR& to) const;

    void DrawInventoryUI(void) const;

    int GetItemSlotSize(Item::TYPE type) const;

    int GetUsedInventorySlotCount(void) const;

private:

    // HLSLライト
    LightEffect lightEffect_;

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
    std::vector<CeilingLight*> ceilingLights_;
    std::vector<VECTOR> ceilingLightBeamPositions_;

    // ワープスター
    std::vector<WarpStar*> warpStars_;

    // 空のPlanet
    Planet* nullPlanet = nullptr;

    float step_;

    // 最初の惑星
    void MakeMainStage(void);

    // アイテム
    std::vector<Item*> items_;

    int itemCount_[(int)Item::TYPE::MAX];
    int registeredItemCount_[(int)Item::TYPE::MAX];
    int stolenItemCount_[(int)Item::TYPE::MAX];

    static constexpr int MAX_ITEM_COUNT = 4;

    int lookingItemIndex_;
    bool isItemMax_;

    // アイテム出現候補地点
    std::vector<VECTOR> itemSpawnPoints_;
    std::vector<VECTOR> watchSpawnPoints_;

    void CreateItem(
        Item::TYPE type,
        ResourceManager::SRC modelSrc,
        VECTOR pos,
        VECTOR scl
    );
    // ランダムにアイテムを出現
    void CreateRandomLaptopItemsFromSpawnPoints(int count);
    void CreateRandomWatchItemsFromSpawnPoints(int count);

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

    // 天井ライト
    void DrawCeilingLightBeams(void);
    void DrawOneCeilingLightBeam(const VECTOR& lightPos);

    int beamGraph_ = -1;

    void CreateBeamGraph(void);
    void CreateKitchenLight(const FurnitureData& data);

    void DrawDisc3D(
        const VECTOR& center,
        float radius,
        int div,
        int color
    );

    void UpdateFlashLightForShader(
        const VECTOR& cameraPos,
        const VECTOR& cameraTarget
    );

    // =========================
    // ポストアウトライン用
    // =========================

    int outlineRTColor_ = -1;
    int outlineRTNormal_ = -1;
    int outlineRTDepth_ = -1;

    int outlinePostPS_ = -1;

    bool InitPostOutline(void);
    void ReleasePostOutline(void);

    void DrawOpaqueSceneForOutline(
        const VECTOR& cameraPos,
        const VECTOR& cameraTarget
    );

    // 重要：
    // DX_SCREEN_BACKに直接描かないため、出力先を受け取る
    void DrawPostOutline(int outputScreen);

    // ミニマップ
    int miniMapScreen_;
    bool isMiniMapVisible_;

    void DrawMiniMap(void) const;
  
    struct MiniMapRect
    {
        int left;
        int top;
        int right;
        int bottom;
    };

    std::vector<MiniMapRect> floorRects;
};
