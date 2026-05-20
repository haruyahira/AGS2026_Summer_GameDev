#pragma once
#include <vector>
#include "../Common/Transform.h"
#include "../Collider/BoxCollider.h"
#include "../../Manager/ResourceManager.h"
#include "../Name.h"

class Furniture {
public:


    // コンストラクタで必要な情報を全部もらう
    Furniture(NAME name, const Transform* trans);

    virtual  void Init(void);

    virtual void Update(void);

    virtual void Draw(void);
    //    trans_.Draw(); // モデルを描画する関数を呼ぶ

    // ゲッター（後で位置などを取得したくなった時のため）
    const Transform& GetTransform() const { return trans_; }

    // 外部（プレイヤーなど）から当たり判定をチェックするための関数
    const std::vector<BoxCollider>& GetColliders() const { return colliders_; }
private:
   
protected:
    NAME      name_;  // 家具の名前（ID）
    Transform trans_; // 位置・回転・スケール・モデル・コライダを保持
    std::vector<BoxCollider> colliders_;
};