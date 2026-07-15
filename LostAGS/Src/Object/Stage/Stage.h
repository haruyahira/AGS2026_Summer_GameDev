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
#include "../../Shader/RimLightEffect.h"

class ResourceManager;
class WarpStar;
class Planet;
class Player;
class Wall;
class Button;

class Stage
{
public:

    // ステージの切り替え間隔
    static constexpr float TIME_STAGE_CHANGE = 1.0f;

    enum class LIGHT_AREA
    {
        NONE = 0,
        HALL = 1 << 0,  // 客席
        KITCHEN = 1 << 1,  // 厨房
        CORRIDOR = 1 << 2,  // 廊下
        BREAKROOM = 1 << 3,  // 休憩室
        LOCKER = 1 << 4,  // 更衣室
        OFFICE = 1 << 5,  // 店長室
        ESCAPE = 1 << 6,  // 脱出エリア
        ALL = 0xFFFF
    };


    struct FurnitureData
    {
        ResourceManager::SRC modelSrc;
        Vector3 pos;
        Vector3 scl;
        Vector3 rot;

        float doorHingeSide = 1.0f;
        float doorOpenSign = -1.0f;

        int lightAreaMask = (int)LIGHT_AREA::NONE;
        bool isLightBlocker = false;
    };

    void StartEscapeTimer(void);

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
    bool IsPickupLineBlocked(const VECTOR& from, const VECTOR& to) const;
public:
    bool IsEmergencyEscape(void) const;

private:

    // HLSLライト
    LightEffect lightEffect_;
    RimLightEffect rimLightEffect_;

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
    std::vector<VECTOR> bookSpawnPoints_;

    void CreateItem(
        Item::TYPE type,
        ResourceManager::SRC modelSrc,
        VECTOR pos,
        VECTOR scl,
        VECTOR rot = VGet(0.0f, 0.0f, 0.0f)
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

    void DebugDrawPickupRange(void) const;
    void DebugDrawItemPickupCheck(void) const;
    
    int escapeTimeLimit_;          // 通常脱出時間 5分
    int emergencyEscapeTime_;      // 予備脱出装置起動まで 1分
    int escapeStartTime_;          // ゲーム開始時間

    bool isEmergencyEscape_;       // 5分経過後か
    bool isResultChanged_;         // リザルトへ移動済みか

    void DrawEscapeTimeUI(void) const;
    void DrawEmergencyEscapeUI(void) const;

    bool isEscapeTimerStarted_;

    Button* escapeButton_;
    VECTOR escapeButtonPos_;
    bool isEscapeButtonActivated_;
    void UpdateEscapeButton();
    void ActivateEscapeButton();
    void DrawPickupUI(void) const;

 // =========================
// ライト遮蔽用：エリア別の壁
// =========================
    std::vector<LightBlocker*> hallBlockers_;
    std::vector<LightBlocker*> kitchenBlockers_;
    std::vector<LightBlocker*> corridorBlockers_;
    std::vector<LightBlocker*> lockerBlockers_;
    std::vector<LightBlocker*> officeBlockers_;
    std::vector<LightBlocker*> escapeBlockers_;
    void RegisterLightBlockerByArea(
        LightBlocker* blocker,
        const VECTOR& pos
    );

    const std::vector<LightBlocker*>& GetLightBlockersByLightPos(
        const VECTOR& lightPos
    ) const;

    bool IsNearEscapeButton(void) const;

    void DrawEscapeButtonUI(void) const;

    // アイテム登録完了UI
    bool isRegisterItemUIVisible_;
    int registerItemUIStartTime_;
    int registerItemUIDuration_;

    int lastRegisterItemCount_;
    int lastRegisterMoney_;
    int lastRegisterItemCounts_[(int)Item::TYPE::MAX];

    void DrawRegisterItemUI(void) const;
    void DrawStoneDeviceRegisterUI(void) const;
    void DrawFlashLightUI(void) const;

};
