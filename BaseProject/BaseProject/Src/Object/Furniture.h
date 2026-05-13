#pragma once
#include "Common/Transform.h"
#include "../Manager/ResourceManager.h"
#include "Name.h"

class Furniture {
public:


    // コンストラクタで必要な情報を全部もらう
    Furniture(NAME name, const Transform* trans);

    void Init(void);

    void Update(void);

    void Draw(void);
    //    trans_.Draw(); // モデルを描画する関数を呼ぶ

    // ゲッター（後で位置などを取得したくなった時のため）
    const Transform& GetTransform() const { return trans_; }

private:
    NAME      name_;  // 家具の名前（ID）
    Transform trans_; // 位置・回転・スケール・モデル・コライダを保持
};