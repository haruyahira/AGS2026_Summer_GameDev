// ColliderSphere.h
#pragma once
#include "ColliderBase.h"
#include <DxLib.h>

class Transform;

class ColliderSphere : public ColliderBase
{
public:
    ColliderSphere(TAG tag, const Transform* follow, const VECTOR& localPos, float radius);
    ~ColliderSphere(void);

    // 中心座標（ワールド）
    VECTOR GetCenter(void) const;

    // 半径
    float GetRadius(void) const { return radius_; }
    void SetRadius(float radius);

    // ★ 弾など、Transform を使わずワールド座標を直指定したいとき用
    void SetPosition(const VECTOR& worldPos);
    void DrawDebug(int color) override;
protected:
 

private:
    VECTOR localPos_;   // Transform 追従時のローカル座標
    float  radius_;

    // ★ follow_ == nullptr のときに使う「ワールド座標そのもの」
    VECTOR worldPos_;
};
