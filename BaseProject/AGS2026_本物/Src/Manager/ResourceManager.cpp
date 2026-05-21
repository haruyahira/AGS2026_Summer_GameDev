#include <DxLib.h>
#include <memory>
#include <assert.h>
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
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
	//---------------------------------------------------------------------------

	// 家具
	// 机
	resourcesMap_.emplace(SRC::F_TABLE,
		std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/F_Table.mv1"));

	// 床
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/floor.mv1");
	resourcesMap_.emplace(SRC::MAIN_PLANET, res);
	// 壁
	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/Wall.mv1");
	resourcesMap_.emplace(SRC::WALL, res);



	//// 吹き出し
	//res = new RES(RES_T::IMG, PATH_IMG + "SpeechBalloon.png");
	//resourcesMap_.emplace(SRC::SPEECH_BALLOON, res);

	// プレイヤー
	res = new RES(RES_T::MODEL, PATH_MDL + "Player/Player.mv1");
	resourcesMap_.emplace(SRC::PLAYER, res);

	// プレイヤー影
	res = new RES(RES_T::IMG, PATH_IMG + "Shadow.png");
	resourcesMap_.emplace(SRC::PLAYER_SHADOW, res);


	// 敵
	res = new RES(RES_T::MODEL, PATH_MDL + "Enemy/Enemy.mv1");
	resourcesMap_.emplace(SRC::ENEMYNORMAL, res);


	// スカイドーム
	res = new RES(RES_T::MODEL, PATH_MDL + "SkyDome/SkyDome.mv1");
	resourcesMap_.emplace(SRC::SKY_DOME, res);

	

	// 落とし穴の惑星
	res = new RES(RES_T::MODEL, PATH_MDL + "Planet/FallPlanet.mv1");
	resourcesMap_.emplace(SRC::FALL_PLANET, res);

	// 平坦な惑星01
	res = new RES(RES_T::MODEL, PATH_MDL + "Planet/FlatPlanet01.mv1");
	resourcesMap_.emplace(SRC::FLAT_PLANET_01, res);

	// 平坦な惑星02
	res = new RES(RES_T::MODEL, PATH_MDL + "Planet/FlatPlanet02.mv1");
	resourcesMap_.emplace(SRC::FLAT_PLANET_02, res);

	// 最後の惑星
	res = new RES(RES_T::MODEL, PATH_MDL + "Planet/LastPlanet.mv1");
	resourcesMap_.emplace(SRC::LAST_PLANET, res);

	// 特別な惑星
	res = new RES(RES_T::MODEL, PATH_MDL + "Planet/RoadPlanet.mv1");
	resourcesMap_.emplace(SRC::SPECIAL_PLANET, res);

	// 足煙
	res = new RES(RES_T::EFFEKSEER, PATH_EFF + "Smoke/Smoke.efkefc");
	resourcesMap_.emplace(SRC::FOOT_SMOKE, res);

	// タンク
	res = new RES(RES_T::MODEL, PATH_MDL + "Tank/Body.mv1");
	resourcesMap_.emplace(SRC::TANK_BODY, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Tank/Wheel.mv1");
	resourcesMap_.emplace(SRC::TANK_WHEEL, res);
	res = new RES(RES_T::MODEL, PATH_MDL + "Tank/Barrel.mv1");
	resourcesMap_.emplace(SRC::TANK_BARREL, res);

}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	for (auto& res : resourcesMap_)
	{
		res.second->Release();
		/*delete res.second;*/
	}
	resourcesMap_.clear();
	delete instance_;
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
		// 登録されていない
		return dummy_;
	}

	// ロード処理
	rPair->second->Load();

	// 念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;
}
