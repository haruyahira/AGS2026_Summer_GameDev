//#include <DxLib.h>
//#include "../../Manager/SceneManager.h"
//#include "AnimationController.h"
//
//AnimationController::AnimationController(int modelId)
//	:
//	modelId_(modelId),
//	playType_(-1),
//	playAnim_(),
//	isLoop_(true)
//{
//}
//
//AnimationController::~AnimationController(void)
//{
//}
//
//void AnimationController::Add(int type, float speed, const std::string path)
//{
//	Animation animation;
//	animation.model = MV1LoadModel(path.c_str());
//	animation.animIndex = -1;
//
//	Add(type, speed, animation);
//}
//
//void AnimationController::AddInFbx(int type, float speed, int animIndex)
//{
//	Animation animation;
//	animation.model = -1;
//	animation.animIndex = animIndex;
//
//	Add(type, speed, animation);
//}
//
//void AnimationController::Play(int type, bool isLoop)
//{
//
//	if (playType_ == type)
//	{
//		// 同じアニメーションだったら再生を継続する
//		return;
//	}
//
//	if (playType_ != -1)
//	{
//		// モデルからアニメーションを外す
//		//MV1DetachAnim(modelId_, playAnim_.attachNo);
//		playAnim_.prevAttachNo = playAnim_.attachNo;
//		MV1SetAttachAnimBlendRate(modelId_, playAnim_.prevAttachNo, 1.0f);
//	
//	}
//
//	// アニメーション種別を変更
//	playType_ = type;
//	playAnim_ = animations_[type];
//
//	// 初期化
//	playAnim_.step = 0.0f;
//	playAnim_.blendRate = 0.0f;
//
//	// モデルにアニメーションを付ける
//	if (playAnim_.model == -1)
//	{
//		// モデルと同じファイルからアニメーションをアタッチする
//		playAnim_.attachNo = MV1AttachAnim(modelId_, playAnim_.animIndex);
//	}
//	else
//	{
//		// 別のモデルファイルからアニメーションをアタッチする
//		// DxModelViewerを確認すること(大体0か1)
//		int animIdx = 0;
//		playAnim_.attachNo = MV1AttachAnim(modelId_, animIdx, playAnim_.model);
//	}
//
//	// アニメーション総時間の取得
//	playAnim_.totalTime = MV1GetAttachAnimTotalTime(modelId_, playAnim_.attachNo);
//
//	// アニメーションループ
//	isLoop_ = isLoop;
//
//}
//
//void AnimationController::Update(void)
//{
//
//	// 経過時間の取得
//	float deltaTime = SceneManager::GetInstance().GetDeltaTime();
//
//	// 再生
//	playAnim_.step += (deltaTime * playAnim_.speed);
//
//	// アニメーションが終了したら
//	if (playAnim_.step > playAnim_.totalTime)
//	{
//		if (isLoop_)
//		{
//			// ループ再生
//			playAnim_.step = 0.0f;
//		}
//		else
//		{
//			// ループしない
//			playAnim_.step = playAnim_.totalTime;
//		}
//	}
//
//	// アニメーション設定
//	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);
//	// ▼ フェード補間
//	if (playAnim_.blendRate < 1.0f)
//	{
//		playAnim_.blendRate += deltaTime * playAnim_.blendSpeed;
//		if (playAnim_.blendRate > 1.0f) playAnim_.blendRate = 1.0f;
//
//		// 新アニメの割合
//		MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, playAnim_.blendRate);
//
//		// 旧アニメの割合
//		if (playAnim_.prevAttachNo != -1)
//		{
//			MV1SetAttachAnimBlendRate(modelId_, playAnim_.prevAttachNo,
//				1.0f - playAnim_.blendRate);
//
//			// 完全に切り替わったら外す
//			if (playAnim_.blendRate >= 1.0f)
//			{
//				MV1DetachAnim(modelId_, playAnim_.prevAttachNo);
//				playAnim_.prevAttachNo = -1;
//			}
//		}
//	}
//
//}
//
//void AnimationController::Release(void)
//{
//
//	// 外部FBXのモデル(アニメーション)解放
//	for (const std::pair<int, Animation>& pair : animations_)
//	{
//		if (pair.second.model != -1)
//		{
//			MV1DeleteModel(pair.second.model);
//		}
//	}
//
//	// 可変長配列をクリアする
//	animations_.clear();
//
//}
//
//int AnimationController::GetPlayType(void) const
//{
//	return playType_;
//}
//
//bool AnimationController::IsEnd(void) const
//{
//
//	bool ret = false;
//
//	if (isLoop_)
//	{
//		// ループ設定されているなら、
//		// 無条件で終了しないを返す
//		return ret;
//	}
//
//	if (playAnim_.step >= playAnim_.totalTime)
//	{
//		// 再生時間を過ぎたらtrue
//		return true;
//	}
//
//	return ret;
//
//}
//
//const AnimationController::Animation& AnimationController::GetPlayAnim(void) const
//{
//	return playAnim_;
//}
//
//void AnimationController::Add(int type, float speed, Animation& animation)
//{
//	animation.speed = speed;
//
//	if (animations_.count(type) == 0)
//	{
//		// 追加
//		animations_.emplace(type, animation);
//	}
//}
#include <DxLib.h>
#include "../../Manager/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
    :
    modelId_(modelId)
{
}

AnimationController::~AnimationController()
{
}

void AnimationController::Add(int type, float speed, const std::string& path)
{
    Animation anim;
    anim.model = MV1LoadModel(path.c_str());
    anim.animIndex = -1;
    anim.speed = speed;
    Add(type, speed, anim);
}

void AnimationController::AddInFbx(int type, float speed, int animIndex)
{
    Animation anim;
    anim.model = -1;
    anim.animIndex = animIndex;
    anim.speed = speed;
    Add(type, speed, anim);
}

void AnimationController::Add(int type, float speed, Animation& anim)
{
    anim.speed = speed;
    animations_[type] = anim;
}

void AnimationController::Play(int type, bool isLoop)
{
    if (playBlend_ < 1.0f)
        return;

    // 同じアニメなら何もしない
    if (playType_ == type)
        return;

    // 旧アニメを保存（あとでフェードアウト）
    prevAttachNo_ = playAttachNo_;

    // 新アニメ情報
    Animation& templ = animations_[type];
    playType_ = type;

    // attach
    if (templ.model == -1)
        playAttachNo_ = MV1AttachAnim(modelId_, templ.animIndex);
    else
        playAttachNo_ = MV1AttachAnim(modelId_, 0, templ.model);

    playTotal_ = MV1GetAttachAnimTotalTime(modelId_, playAttachNo_);

    // 再生ステータス初期化
    playStep_ = 0.0f;
    playBlend_ = 0.0f;

    isLoop_ = isLoop;
}

void AnimationController::Update()
{
    if (playType_ == -1) return;

    float dt = SceneManager::GetInstance().GetDeltaTime();
    Animation& templ = animations_[playType_];

    // 再生時間進行
    playStep_ += dt * templ.speed;

    // 終了処理
    if (playStep_ >= playTotal_) {
        if (isLoop_)
            playStep_ = 0.0f;
        else
            playStep_ = playTotal_;
    }

    // アニメ適用
    MV1SetAttachAnimTime(modelId_, playAttachNo_, playStep_);

    // クロスフェード（新→旧）
    if (playBlend_ < 1.0f) {

        playBlend_ += dt * templ.blendSpeed;
        if (playBlend_ > 1.0f) playBlend_ = 1.0f;

        // 新アニメ：徐々に 1.0 へ
        MV1SetAttachAnimBlendRate(modelId_, playAttachNo_, playBlend_);

        // 旧アニメ：1 → 0
        if (prevAttachNo_ != -1) {

            MV1SetAttachAnimBlendRate(modelId_, prevAttachNo_, 1.0f - playBlend_);

            // 完全に切り替わったら外す
            if (playBlend_ >= 1.0f) {
                MV1DetachAnim(modelId_, prevAttachNo_);
                prevAttachNo_ = -1;
            }
        }
    }
}

void AnimationController::Release()
{
    for (auto& it : animations_) {
        if (it.second.model != -1) {
            MV1DeleteModel(it.second.model);
        }
    }
    animations_.clear();
}

int AnimationController::GetPlayType() const
{
    return playType_;
}

bool AnimationController::IsEnd() const
{
    if (isLoop_) return false;

    return playStep_ >= playTotal_;
}
