#pragma once
#include <map>
#include <string>
#include <memory>
#include "Resource.h"

class ResourceManager
{

public:

	// リソース名
	enum class SRC
	{
		BACK_GROUND,
		TITLE_LOGO,
		TITLE_READ_PANDA,
		TITLE_SELECT_STRAT,
		TITLE_SELECT_RANKING,
		TITLE_SELECT_CLOSE,
		TITLE_SELECT,
		TITLE_SELECT2,
		TITLE_SELECT_BRIGHT1,
		TITLE_SELECT_BRIGHT2,
		TITLE_SELECT_BRIGHT3,

		ENEMYNORMAL,
		ENEMYPOLICE,

		F_TABLE,
		WALL,
		CEILING_LIGHT,
		F_G,
		F_F,
		BOOKSLF, // 本棚
		SHELF, // 棚
		SHELFDOOR, // 扉つきの棚
		LOCKER, // ロッカー
		FREEZER, // 冷凍庫
		BOOK, // 本
		DOOR, // ドア
		MONITOR, // モニター
		DESKTOPPC, // デスクトップPC
		SINK, // シンク
		BUTTON, // ボタン
		TRASHCAN, // ゴミ箱



		RETURN_POINT,
		// アイテム
		LAPTOP,// ノートPC
		WATCH, // 腕時計
		BOOK168, // １６８Book
		SMARTPHONE, // スマートフォン
		CREDITCARD, // クレジットカード


		// Sound
		WALK_SE,
		WALK_E_SE,
		RUN_SE,
		ATTACK_SE,
		HIT_SE,
		DISC_SE,
		DOOROP_SE,
		GAME_BGM,
		CHASE_BGM,
		TITLE_BGM,
		CLEAR_BGM,



		SPEECH_BALLOON,
		PLAYER,
		PLAYER_SHADOW,
		SKY_DOME,
		FLOOR,
		FALL_PLANET,
		FLAT_PLANET_01,
		FLAT_PLANET_02,
		LAST_PLANET,
		SPECIAL_PLANET,
		FOOT_SMOKE,
		WARP_STAR,
		WARP_STAR_ROT_EFF,
		WARP_ORBIT,
		BLACK_HOLE,
		GOAL_STAR,
		CLEAR,
		TANK_BODY,
		TANK_BARREL,
		TANK_WHEEL,
	};

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static ResourceManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切替時に一旦解放)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	// リソースのロード
	const Resource& Load(SRC src);

	// リソースの複製ロード(モデル用)
	int LoadModelDuplicate(SRC src);

	void DeleteDuplicateModel(int& handle);


private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理の対象
	//std::map<SRC, Resource*> resourcesMap_;
	std::map<SRC, std::unique_ptr<Resource>> resourcesMap_;

	// 読み込み済みリソース
	std::map<SRC, Resource*> loadedMap_;

	Resource dummy_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(SRC src);


};
