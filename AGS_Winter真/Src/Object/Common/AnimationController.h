//#pragma once
//#include <string>
//#include <map>
//
//class AnimationController
//{
//
//public:
//
//	// アニメーションデータ
//	struct Animation
//	{
//		int model = -1;
//		int attachNo = -1;
//		int animIndex = 0;
//		float speed = 0.0f;
//		float totalTime = 0.0f;
//		float step = 0.0f;
//		float blendRate = 1.0f;
//		float blendSpeed = 4.0f; // 大きいほど速く切り替わる
//		int prevAttachNo = -1;   // 前のアニメ
//	};
//
//	// 既定のコンストラクターを追加
//	AnimationController() = default;
//
//	// コンストラクタ
//	AnimationController(int modelId);
//
//	// デストラクタ
//	~AnimationController(void);
//
//	// 外部FBXからアニメーション追加
//	void Add(int type, float speed, const std::string path);
//	
//	// 同じFBX内のアニメーションを準備
//	void AddInFbx(int type, float speed, int animIndex);
//
//	// アニメーション再生
//	void Play(int type, bool isLoop = true);
//
//	// 更新
//	void Update(void);
//
//	// 解放
//	void Release(void);
//
//	// 再生中のアニメーション
//	int GetPlayType(void) const;
//
//	// 再生終了
//	bool IsEnd(void) const;
//
//	// 再生中のアニメーション情報を取得
//	const Animation& GetPlayAnim(void) const;
//
//	// 追加: ルートモーションの使用設定
//	void SetUseRootMotion(bool useRootMotion);
//
//private:
//
//	// アニメーションするモデルのハンドルID
//	int modelId_;
//
//	
//
//	// 種類別のアニメーションデータ
//	std::map<int, Animation> animations_;
//
//	// 再生中のアニメーション
//	int playType_;
//	Animation playAnim_;
//
//	// アニメーションをループするかしないか
//	bool isLoop_;
//
//	// アニメーション追加の共通処理
//	void Add(int type, float speed, Animation& animation);
//};
//
#pragma once
#include <string>
#include <map>

class AnimationController
{
public:

    // アニメ定義（テンプレート）
    struct Animation
    {
        int model = -1;      // 外部FBX用（なければ -1）
        int animIndex = 0;   // 同一FBX内アニメ番号
        float speed = 1.0f;
        float blendSpeed = 3.0f;
    };

    AnimationController() = default;
    AnimationController(int modelId);
    ~AnimationController();

    void Add(int type, float speed, const std::string& path);
    void AddInFbx(int type, float speed, int animIndex);

    void Play(int type, bool isLoop = true);
    void Update();
    void Release();

    int GetPlayType() const;
    bool IsEnd() const;

private:

    int modelId_ = -1;

    std::map<int, Animation> animations_;

    // 再生状態（テンプレートから切り離した）
    int playType_ = -1;
    int playAttachNo_ = -1;
    int prevAttachNo_ = -1;

    float playStep_ = 0.0f;
    float playBlend_ = 1.0f;
    float playTotal_ = 0.0f;

    bool isLoop_ = true;

    void Add(int type, float speed, Animation& anim);
};
