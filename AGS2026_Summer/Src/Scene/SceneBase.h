#pragma once
class ResourceManager;

class SceneBase
{
public:
	// コンストラクタ
	SceneBase(void);

	// デストラクタ
	virtual ~SceneBase(void) = 0;
	/*
	* 学習メモ
	* virtualは「この関数は子クラスで上書きされる可能性がある」という意味
	* =0は「この関数は純粋仮想関数で、子クラスで必ず上書きされなければならない」という意味
	*/

	// 初期化処理
	virtual void Init(void) = 0;

	// 更新ステップ
	virtual void Update(void) = 0;

	// 描画処理
	virtual void Draw(void) = 0;

protected:

	// リソース管理
	ResourceManager& resMng_;

	int screenX_, screenY_; // 画面サイズ

};

