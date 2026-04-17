#pragma once
#include <map>
#include <string>
#include "Resource.h"
#include <unordered_map>

class ResourceManager
{

public:

	// リソース名
	enum class SRC
	{
		TITLE,
		TITLE_2560,
		PUSH_SPACE,
		PIT_FALL_PLANET,
		SPHERE_PLANET,
		PLAYER,
		PLAYER_SHADOW,
		KATANA,
		STAGE,
		SKYDOME,
		ENEMY,
		SHOT,
		ROCK,

		OP,
		ED,
		BGM1,
		BGM2,
		

		WALK,
		RUN,

		BAKUHATU
	};

	enum class SRC2
	{
		BAKUHATU
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

private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理の対象
	std::map<SRC, Resource*> resourcesMap_;

	// 読み込み済みリソース
	std::map<SRC, Resource*> loadedMap_;
	//std::map<SRC, Resource&> loadedMap_;


	Resource dummy_;
	Resource* dummyPtr_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にするa
	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(SRC src);
	

};
