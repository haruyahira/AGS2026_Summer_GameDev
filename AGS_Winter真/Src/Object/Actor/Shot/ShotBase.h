#pragma once
#include <vector>
#include <DxLib.h>
#include "../../Collider/ColliderSphere.h"

// 弾の基底クラス
class ShotBase
{
public:
    // 弾の種類
    enum class TYPE
    {
        STRAIGHT,
        SWORD,
        SHOT
    };

    // 標準の自己発光色
    static constexpr COLOR_F COLOR_EMI_DEFAULT = { 0.6f, 0.6f, 0.6f, 1.0f };

    // コンストラクタ(元となるモデルのハンドルID)
    ShotBase(TYPE type, int baseModelId);

    // デストラクタ
    virtual ~ShotBase(void);

    // 弾の生成(表示開始座標、弾の進行方向)
    virtual void CreateShot(VECTOR pos, VECTOR dir);

    // 更新ステップ
    virtual void Update(void);

    // 描画
    void Draw(void);

    // 解放処理
    void Release(void);

    // 生存判定
    bool IsAlive(void) const { return isAlive_; }
    bool IsDead()   const { return !isAlive_; }

    // 弾の消去
    void Kill() { isAlive_ = false; shotPos.clear(); }

    // 弾の座標
    VECTOR GetPos(void) const { return pos_; }
    const std::vector<VECTOR>& Getpos(void) const { return shotPos; }

    // 弾の衝突判定用半径
    float GetCollisionRadius(void) const { return collisionRadius_; }

    // コライダ取得（プレイヤー→敵の当たり判定用）
    ColliderSphere* GetCollider() const { return collider_; }

    // 弾の種別
    TYPE GetType(void) const { return type_; }

    // 浮遊モード切替（剣専用。基底では何もしない）
    virtual void SetFloatMode(bool) {}

protected:
    // 弾の種別
    TYPE type_;

    // 弾のモデルID
    int modelId_;

    // 方向
    VECTOR dir_;
    // 弾の大きさ
    VECTOR scales_;
    // 弾の座標（最新）
    VECTOR pos_;
    // 表示用／当たり判定用に保持している座標リスト
    std::vector<VECTOR> shotPos;

    // コライダ
    ColliderSphere* collider_;

    // 弾の移動速度
    float speed_;

    // 弾の生存判定
    bool isAlive_;
    // 弾の生存期間
    int cntAlive_;

    // 弾の衝突判定用半径
    float collisionRadius_;
    float radius_ = 20.0f;   // デフォルトの当たり判定半径

    // パラメータ設定（派生クラスで実装）
    virtual void SetParam(void) = 0;

    // 弾の生存期間の減少
    void ReduceCntAlive(void);

    // 基本移動
    virtual void Move(void);

    // 描画本体
    virtual void DrawModel(void);
};

