#pragma once
#include <DxLib.h>
#include "ColliderBase.h"
class Transform;
class ColliderCapsule : public ColliderBase
{
public:
	// コンストラクタ
	ColliderCapsule(
		TAG tag, const Transform* follow,
		const VECTOR& localPosTop, const VECTOR& localPosDown, float radius);
	// デストラクタ
	~ColliderCapsule(void);
	// 親Transformからの相対位置を取得
	const VECTOR& GetLocalPosTop(void) const;
	const VECTOR& GetLocalPosDown(void) const;
	// 親Transformからの相対位置をセット
	void SetLocalPosTop(const VECTOR& pos);
	void SetLocalPosDown(const VECTOR& pos);
	// ワールド座標を取得
	VECTOR GetPosTop(void) const;
	VECTOR GetPosDown(void) const;
	// 半径
	float GetRadius(void) const;
	void SetRadius(float radius);
	// 高さ
	float GetHeight(void) const;
	// カプセルの中心座標
	VECTOR GetCenter(void) const;
protected:
	// デバッグ用描画
	void DrawDebug(int color) override;
private:
	// 親Transformからの相対位置(上側)
	VECTOR localPosTop_;
	// 親Transformからの相対位置(下側)
	VECTOR localPosDown_;
	// 半径
	float radius_;
};