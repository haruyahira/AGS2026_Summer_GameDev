#include "Locker.h"
#include "../../Manager/InputManager.h"

// コンストラクタ
Locker::Locker(const Transform* trans)
    : Furniture(NAME::BOOKSLF, trans) {
}

// 初期化
void Locker::Init() {
    colliders_.clear();

    // アニメーションをモデルにアタッチ
    // 第2引数はアニメーション番号
    // 0番で動かない場合は 1, 2, 3... と変えて確認
    animAttachIndex_ = MV1AttachAnim(trans_.modelId, 0);

    // アニメーションの総再生時間を取得
    if (animAttachIndex_ != -1) {
        animTotalTime_ = MV1GetAttachAnimTotalTime(trans_.modelId, animAttachIndex_);
        animTime_ = 0.0f;
    }
}

// 更新
void Locker::Update(void) {
    if (animAttachIndex_ == -1) {
        return;
    }

    // アニメーション時間を進める
    animTime_ += animSpeed_;

    //// 最後まで再生したら最初に戻す
    //if (animTime_ >= animTotalTime_) {
    //    animTime_ = 0.0f;
    //}

    // 現在のアニメーション時間をモデルに反映
    MV1SetAttachAnimTime(trans_.modelId, animAttachIndex_, animTime_);
}

void Locker::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);
}