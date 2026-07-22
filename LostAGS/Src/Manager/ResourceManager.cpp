#include <DxLib.h>
#include <memory>
#include <assert.h>
#include "../Application.h"
#include "../Utility/DebugMemory.h"
#include "../Utility/DebugConsole.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{

	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
		instance_->Init();
	}

}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{

	// 推奨しませんが、どうしても使いたい方は
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	Resource* res;

	// タイトル関連--------------------------------------------------------------
	// 画像
	// タイトル背景画像
	resourcesMap_.emplace(SRC::BACK_GROUND,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleBackground.png"));
	// タイトルロゴ
	resourcesMap_.emplace(SRC::TITLE_LOGO,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleLogo.png"));
	// レッサーパンダ
	resourcesMap_.emplace(SRC::TITLE_READ_PANDA,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleRedpanda2.png"));
	// 選択肢
	resourcesMap_.emplace(SRC::TITLE_SELECT,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleSelectNormal.png"));
	// 選択肢当たり判定用
	resourcesMap_.emplace(SRC::TITLE_SELECT2,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleSelectNormal2.png"));
	// 選択肢（光るスタート）
	resourcesMap_.emplace(SRC::TITLE_SELECT_BRIGHT1,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleSelectBright1.png"));
	// 選択肢（光るランキング）
	resourcesMap_.emplace(SRC::TITLE_SELECT_BRIGHT2,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleSelectBright2.png"));
	// 選択肢（光る終了）
	resourcesMap_.emplace(SRC::TITLE_SELECT_BRIGHT3,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "TitleSelectBright3.png"));
	// 選択中のボタン
	resourcesMap_.emplace(SRC::TITLE_SELECT_ICON,
		std::make_unique<RES>(RES_T::IMG, PATH_IMG + "SelectIcon.png"));
	//---------------------------------------------------------------------------

	// 家具
	// 机
	resourcesMap_.emplace(SRC::F_TABLE,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/F_Table.mv1"));
	
	// 本棚
	resourcesMap_.emplace(SRC::BOOKSLF,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/BookShelf.mv1"));
	// 棚
	resourcesMap_.emplace(SRC::SHELF,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Shelf.mv1"));
	// 扉つきの棚
	resourcesMap_.emplace(SRC::SHELFDOOR,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/ShelfDoor.mv1"));

	// ロッカー
	resourcesMap_.emplace(SRC::LOCKER,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Locker.mv1"));	
	// 冷凍庫
	resourcesMap_.emplace(SRC::FREEZER,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Freezer.mv1"));	

	// 床
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/floor.mv1");
	resourcesMap_.emplace(SRC::FLOOR, res);
	// 天井ライト
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/CeilingLight.mv1");
	resourcesMap_.emplace(SRC::CEILING_LIGHT, res);
	// 壁
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Wall.mv1");
	resourcesMap_.emplace(SRC::WALL, res);
	// ドア
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Door.mv1");
	resourcesMap_.emplace(SRC::DOOR, res);
	// 脱出装置
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Button.mv1");
	resourcesMap_.emplace(SRC::BUTTON, res);
	// ゴミ箱
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Trashcan.mv1");
	resourcesMap_.emplace(SRC::TRASHCAN, res);
	// モニター
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Wall.mv1");
	resourcesMap_.emplace(SRC::WALL, res);
	// デスクトップPC
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Wall.mv1");
	resourcesMap_.emplace(SRC::WALL, res);
	// シンク
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Wall.mv1");
	resourcesMap_.emplace(SRC::WALL, res);
	// 本
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Book.mv1");
	resourcesMap_.emplace(SRC::BOOK, res);
	// ショーケース
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Showcase_frame.mv1");
	resourcesMap_.emplace(SRC::F_F, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Showcase_grasu.mv1");
	resourcesMap_.emplace(SRC::F_G, res);
	// ゴール地点、納品所
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/ReturnPoint.mv1");
	resourcesMap_.emplace(SRC::RETURN_POINT, res);

	// アイテム------------------------------------------------------------
	res = new RES(RES_T::MODEL, PATH_MDL + "Item/Laptop.mv1"); // ノートPC
	resourcesMap_.emplace(SRC::LAPTOP, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Item/Watch.mv1"); // 腕時計
	resourcesMap_.emplace(SRC::WATCH, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Item/168Book.mv1"); // １６８Book
	resourcesMap_.emplace(SRC::BOOK168, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Item/SmartPhone.mv1"); // スマートフォン
	resourcesMap_.emplace(SRC::SMARTPHONE, res);

	// 登場キャラクター----------------------------------------------------
	res = new RES(RES_T::MODEL, PATH_MDL + "Player/Player.mv1"); // プレイヤー
	resourcesMap_.emplace(SRC::PLAYER, res);
	res = new RES(RES_T::IMG, PATH_IMG + "Shadow.png"); // プレイヤー影
	resourcesMap_.emplace(SRC::PLAYER_SHADOW, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Enemy/Enemy.mv1"); // 敵
	resourcesMap_.emplace(SRC::ENEMYNORMAL, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Enemy/EnemyPolice.mv1"); // 敵
	resourcesMap_.emplace(SRC::ENEMYPOLICE, res);


	//============
	// Sound関連
	//============
	// BGM
	res = new RES(RES_T::SOUND, PATH_SND + "Bgm/GameBgm.wav");
	resourcesMap_.emplace(SRC::GAME_BGM, res);	
	res = new RES(RES_T::SOUND, PATH_SND + "Bgm/TitleBgm.wav");
	resourcesMap_.emplace(SRC::TITLE_BGM, res);
	res = new RES(RES_T::SOUND, PATH_SND + "Bgm/ChaseBgm.wav");
	resourcesMap_.emplace(SRC::CHASE_BGM, res);
	res = new RES(RES_T::SOUND, PATH_SND + "Bgm/ClearBgm.mp3");
	resourcesMap_.emplace(SRC::CLEAR_BGM, res);
	// SE
	res = new RES(RES_T::SOUND, PATH_SND + "Se/Walk.mp3");
	resourcesMap_.emplace(SRC::WALK_SE, res);
	res = new RES(RES_T::SOUND, PATH_SND + "Se/EnemyWalk.mp3");
	resourcesMap_.emplace(SRC::WALK_E_SE, res);
	res = new RES(RES_T::SOUND, PATH_SND + "Se/Attack.mp3");
	resourcesMap_.emplace(SRC::ATTACK_SE, res);
	res = new RES(RES_T::SOUND, PATH_SND + "Se/Hit.mp3");
	resourcesMap_.emplace(SRC::HIT_SE, res);
	res = new RES(RES_T::SOUND, PATH_SND + "SE/Discovery.wav");
	resourcesMap_.emplace(SRC::DISC_SE, res);
	res = new RES(RES_T::SOUND, PATH_SND + "SE/DoorOpen.mp3");
	resourcesMap_.emplace(SRC::DOOROP_SE, res);


}

void ResourceManager::Release(void)
{
#ifdef _DEBUG
	PrintDebugMemoryInfo("Before ResourceManager Release");
#endif

	for (auto& p : loadedMap_)
	{
		if (p.second == nullptr)
		{
			continue;
		}

		// SOUND は解放しない
		if (p.second->type_ == Resource::TYPE::SOUND)
		{
			continue;
		}

#ifdef _DEBUG
		DebugLog(
			"[RESOURCE RELEASE] type:%d handle:%d\n",
			static_cast<int>(p.second->type_),
			p.second->handleId_
		);
#endif

		p.second->Release();
	}

	loadedMap_.clear();

#ifdef _DEBUG
	PrintDebugMemoryInfo("After ResourceManager Release");
#endif
}
void ResourceManager::Destroy(void)
{
#ifdef _DEBUG
	PrintDebugMemoryInfo("Before ResourceManager Destroy");
#endif

	Release();

	for (auto& res : resourcesMap_)
	{
		res.second->Release();
	}

	resourcesMap_.clear();

	delete instance_;
	instance_ = nullptr;

#ifdef _DEBUG
	PrintDebugMemoryInfo("After ResourceManager Destroy");
#endif
}
const Resource& ResourceManager::Load(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{

	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

#ifdef _DEBUG
	g_DebugMemory.modelDuplicateCount++;

	DebugLog(
		"[DUP MODEL] src:%d baseHandle:%d dupHandle:%d DupCount:%d Private:%d MB\n",
		static_cast<int>(src),
		res.handleId_,
		duId,
		g_DebugMemory.modelDuplicateCount,
		GetPrivateMemoryMB()
	);
#endif

	return duId;

}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{
	// ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return *resourcesMap_.find(src)->second;
	}

	// リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
#ifdef _DEBUG
		DebugLog(
			"[LOAD ERROR] src:%d is not registered.\n",
			static_cast<int>(src)
		);
#endif
		return dummy_;
	}

#ifdef _DEBUG
	int beforeMB = GetPrivateMemoryMB();
#endif

	// ロード処理
	rPair->second->Load();

#ifdef _DEBUG
	int afterMB = GetPrivateMemoryMB();
	int diffMB = afterMB - beforeMB;

	Resource& loadedRes = *rPair->second;

	switch (loadedRes.type_)
	{
	case Resource::TYPE::MODEL:
		g_DebugMemory.modelCount++;
		break;

	case Resource::TYPE::IMG:
		g_DebugMemory.graphCount++;
		break;

	case Resource::TYPE::SOUND:
		g_DebugMemory.soundCount++;
		break;

	default:
		break;
	}

	DebugLog(
		"[LOAD RESOURCE] src:%d type:%d path:%s handle:%d diff:%d MB before:%d MB after:%d MB\n",
		static_cast<int>(src),
		static_cast<int>(loadedRes.type_),
		loadedRes.GetPath().c_str(),
		loadedRes.handleId_,
		diffMB,
		beforeMB,
		afterMB
	);
#endif

	loadedMap_.emplace(src, rPair->second.get());

	return *rPair->second;
}

void ResourceManager::DeleteDuplicateModel(int& handle)
{
	if (handle != -1)
	{
		MV1DeleteModel(handle);
		handle = -1;

#ifdef _DEBUG
		if (g_DebugMemory.modelDuplicateCount > 0)
		{
			g_DebugMemory.modelDuplicateCount--;
		}

		DebugLog(
			"[DELETE DUP MODEL] DupCount:%d Private:%d MB\n",
			g_DebugMemory.modelDuplicateCount,
			GetPrivateMemoryMB()
		);
#endif
	}
}