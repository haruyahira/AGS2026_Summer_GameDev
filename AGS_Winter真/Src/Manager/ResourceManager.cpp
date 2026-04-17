#include <DxLib.h>
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

	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_BGM = Application::PATH_BGM;
	

	Resource* res;

	// 画像
	// タイトル画像
	res = new RES(RES_T::IMG, PATH_IMG + "Title.png");
	resourcesMap_.emplace(SRC::TITLE, res);
	res = new RES(RES_T::IMG, PATH_IMG + "Title2560.png");
	resourcesMap_.emplace(SRC::TITLE_2560, res);
	// PushSpace画像
	res = new RES(RES_T::IMG, PATH_IMG + "PushSpace.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, res);

	// モデル
	// プレイヤー
	res = new RES(RES_T::MODEL,
		PATH_MDL + "Player/shiru.mv1");
	resourcesMap_.emplace(SRC::PLAYER, res);
    // ステージ
	res = new RES(RES_T::MODEL,
		PATH_MDL + "Stage/Stage.mv1");
	resourcesMap_.emplace(SRC::STAGE, res); 
	// 刀
	res = new RES(RES_T::MODEL,
		PATH_MDL + "Player/Katana.mv1");
	resourcesMap_.emplace(SRC::KATANA, res);
	// 敵
	res = new RES(RES_T::MODEL,
		PATH_MDL + "Enemy/Enemy.mv1");
	resourcesMap_.emplace(SRC::ENEMY, res);
	// 岩
	res = new RES(RES_T::MODEL,
		PATH_MDL + "Enemy/Rock.mv1");
	resourcesMap_.emplace(SRC::ROCK, res);


	// 通常攻撃用
	res = new RES(RES_T::MODEL,
		PATH_MDL + "Player/Katana.mv1");
	resourcesMap_.emplace(SRC::SHOT, res);

	// BGM
	// OP
	res = new RES(RES_T::BGM,
		PATH_BGM + "op.wav");
	resourcesMap_.emplace(SRC::OP, res);

	// BGM1
	res = new RES(RES_T::BGM,
		PATH_BGM + "bgm1.wav");
	resourcesMap_.emplace(SRC::BGM1, res);
	// BGM2
	res = new RES(RES_T::BGM,
		PATH_BGM + "bgm2.wav");
	resourcesMap_.emplace(SRC::BGM2, res);

	// SE
	res = new RES(RES_T::BGM,
		PATH_BGM + "Walk.wav");
	resourcesMap_.emplace(SRC::WALK, res);
	res = new RES(RES_T::BGM,
		PATH_BGM + "Run.wav");
	resourcesMap_.emplace(SRC::RUN, res);

	// エフェクト
	res = new RES(RES_T::EFFECT,
		PATH_EFF+ "Flame.efk", 100.0f);
	resourcesMap_.emplace(SRC::BAKUHATU, res);


	
}

void ResourceManager::Release(void)
{
	//for (auto& p : loadedMap_)
	//{
	//	p.second.Release();
	//}

	//loadedMap_.clear();
	for (auto& p : loadedMap_)
	{
		p.second->Release();  // ← . ではなく -> を使う
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	for (auto& res : resourcesMap_)
	{
		res.second->Release();
		delete res.second;
	}
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	/*const Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;*/
	return _Load(src);
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
	dummyPtr_ = new Resource(); // 空の Resource を作る
}

Resource& ResourceManager::_Load(SRC src)
{

	//// ロード済みチェック
	//const auto& lPair = loadedMap_.find(src);
	//if (lPair != loadedMap_.end())
	//{
	//	return *resourcesMap_.find(src)->second;
	//}

	//// リソース登録チェック
	//const auto& rPair = resourcesMap_.find(src);
	//if (rPair == resourcesMap_.end())
	//{
	//	// 登録されていない
	//	return dummy_;
	//}

	//// ロード処理
	//rPair->second->Load();

	//// 念のためコピーコンストラクタ
	//loadedMap_.emplace(src, *rPair->second);

	//return *rPair->second;
 // ロード済みチェック
	auto it = loadedMap_.find(src);
	if (it != loadedMap_.end())
	{
		return *(it->second); // ポインタを参照に変換
	}

	// 登録チェック
	auto rIt = resourcesMap_.find(src);
	if (rIt == resourcesMap_.end())
	{
		return *dummyPtr_; // ポインタを参照に変換
	}

	// ロード
	rIt->second->Load();

	// ポインタを保持
	loadedMap_[src] = rIt->second;

	return *(rIt->second);

}


