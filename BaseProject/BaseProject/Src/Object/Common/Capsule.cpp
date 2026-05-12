#include <DxLib.h>
#include "Transform.h"
#include "../../Utility/ColorUtility.h"
#include "Capsule.h"

Capsule::Capsule(const Transform& parent) : transformParent_(parent)
{
	radius_ = 0.0f;
	localPosTop_ = { 0.0f, 0.0f, 0.0f };
	localPosDown_ = { 0.0f, 0.0f, 0.0f };
}

Capsule::Capsule(const Capsule& base, const Transform& parent) : transformParent_(parent)
{
	radius_ = base.GetRadius();
	localPosTop_ = base.GetLocalPosTop();
	localPosDown_ = base.GetLocalPosDown();
}

Capsule::~Capsule(void)
{
}

void Capsule::Update(void)
{
	if (targetModelId_ == -1) return;

	// 頭（Top）の位置をボーンから自動取得
	if (topBoneFrame_ != -1) {
		localPosTop_ = CalculateLocalBonePos(topBoneFrame_);
	}
	// 足元（Down）の位置をボーンから自動取得（指定があれば）
	if (downBoneFrame_ != -1) {
		localPosDown_ = CalculateLocalBonePos(downBoneFrame_);
	}
}		

void Capsule::Draw(void)
{

	// 上の球体
	VECTOR pos1 = GetPosTop();
	DrawSphere3D(pos1, radius_, 5, Color::WHITE, Color::WHITE, false);

	// 下の球体
	VECTOR pos2 = GetPosDown();
	DrawSphere3D(pos2, radius_, 5, Color::WHITE, Color::WHITE, false);

	VECTOR dir;
	VECTOR s;
	VECTOR e;

	// 球体を繋ぐ線(X+)
	dir = transformParent_.GetRight();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, Color::WHITE);

	// 球体を繋ぐ線(X-)
	dir = transformParent_.GetLeft();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, Color::WHITE);

	// 球体を繋ぐ線(Z+)
	dir = transformParent_.GetForward();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, Color::WHITE);

	// 球体を繋ぐ線(Z-)
	dir = transformParent_.GetBack();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, Color::WHITE);

	// カプセルの中心
	DrawSphere3D(GetCenter(), 5.0f, 10, Color::WHITE, Color::WHITE, true);

}

VECTOR Capsule::GetLocalPosTop(void) const
{
	return localPosTop_;
}

VECTOR Capsule::GetLocalPosDown(void) const
{
	return localPosDown_;
}

void Capsule::SetLocalPosTop(const VECTOR& pos)
{
	localPosTop_ = pos;
}

void Capsule::SetLocalPosDown(const VECTOR& pos)
{
	localPosDown_ = pos;
}

VECTOR Capsule::GetPosTop(void) const
{
	return GetRotPos(localPosTop_);
}

VECTOR Capsule::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}

VECTOR Capsule::GetRotPos(const VECTOR& localPos) const
{// 1. ワールド回転とモデルの初期回転(180度など)を合成
	VECTOR rotatedPos = transformParent_.quaRotLocal.PosAxis(localPos);

	// 2. 合成した回転でボーンのローカル座標を回す
	VECTOR localRotPos = transformParent_.quaRot.PosAxis(rotatedPos);
	// 3. プレイヤーの座標を足す
	return VAdd(transformParent_.pos, localRotPos);
}

float Capsule::GetRadius(void) const
{
	return radius_;
}

void Capsule::SetRadius(float radius)
{
	radius_ = radius;
}

float Capsule::GetHeight(void) const
{
	return localPosTop_.y;
}

VECTOR Capsule::GetCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();

	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}

void Capsule::AttachToBone(int modelId, int topFrame, int downFrame)
{
	targetModelId_ = modelId;
	topBoneFrame_ = topFrame;
	downBoneFrame_ = downFrame;
}

VECTOR Capsule::CalculateLocalBonePos(int frameIndex)
{
	// 1. ボーンの「今、画面に見えている世界座標」を取得
	VECTOR boneWorldPos = MV1GetFramePosition(targetModelId_, frameIndex);

	// 2. モデル全体の「現在の配置行列」を取得
	// MV1GetMatrix ではなく、transformParent_ が持っている最新の行列、
	// または MV1GetFrameLocalWorldMatrix(モデルのルート) を検討します。
	// ここでは最も確実な「transformParent_」のワールド行列の逆行列を使います。

	MATRIX modelMatrix = MV1GetMatrix(targetModelId_); // これが古い場合がある

	// 3. 逆行列を作成
	MATRIX invModelMatrix = MInverse(modelMatrix);

	// 4. 座標変換
	VECTOR result = VTransform(boneWorldPos, invModelMatrix);
	return VScale(result, 0.1f);
}